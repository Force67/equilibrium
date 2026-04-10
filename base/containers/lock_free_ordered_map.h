// Copyright (C) 2023-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// LockFreeOrderedHashMap<Key, Value>
//
// A genuinely lock-free hash map that also preserves insertion order.
//   - insert / find / remove are all lock-free (CAS-only on the hot path).
//   - Memory reclamation is handled by epoch-based reclamation (EBR), so
//     readers can never use-after-free a node a writer is removing.
//   - The order list is a Michael & Scott queue keyed off insertion time,
//     spliced atomically alongside the bucket linked list.
//
// EBR design (loosely after CMU's ParlayHash):
//   - Every operation grabs an `ebr::Guard` which "announces" the current
//     global epoch in a per-thread slot.
//   - remove() marks the node deleted and tries to CAS-unlink from the
//     bucket; if that fails it leaves the node tagged-but-still-linked
//     and the next sweep / find will deal with it.
//   - A periodic GC pass walks the bucket + order chains, physically
//     unlinks tagged nodes, stamps each with the current epoch, then frees
//     any node whose retirement epoch is at least 2 ahead of the minimum
//     active epoch — guaranteed unreachable by any concurrent reader.
//
// History: this implementation grew up inside zetanet (znet/fancy_queue.h)
// to support the network packet ACK queue. It's been lifted here so the
// rest of the engine has a single canonical lock-free ordered map and the
// container sits in the layer it logically belongs to.
#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

#include <base/arch.h>
#include <base/atomic.h>
#include <base/containers/vector.h>
#include <base/threading/mutex.h>

namespace base {

// ---------------------------------------------------------------------------
// Epoch-based reclamation (EBR).
// Protects concurrent readers from use-after-free: a node is only freed
// once every thread that could possibly hold a pointer to it has exited
// its read-side critical section.  Based on ParlayHash (CMU).
// ---------------------------------------------------------------------------
namespace ebr {

// Maximum concurrent threads that can hold an EBR guard simultaneously.
// Only live threads consume slots; IDs are recycled when threads exit.
constexpr int kMaxSlots = 4096;

struct alignas(64) AnnounceSlot {
  std::atomic<long> epoch{-1};
};

struct EpochState {
  alignas(64) std::atomic<long> current{0};
  AnnounceSlot slots[kMaxSlots];

  // Thread ID pool — mutex only taken on thread birth/death (not hot path).
  std::mutex pool_mutex;
  std::vector<int> free_ids;
  std::atomic<int> high_watermark{0};

  int acquire_id() {
    std::lock_guard<std::mutex> lk(pool_mutex);
    if (!free_ids.empty()) {
      int id = free_ids.back();
      free_ids.pop_back();
      return id;
    }
    int id = high_watermark.load(std::memory_order_relaxed);
    // Release: ensures try_advance() (which loads with acquire) sees this
    // slot as part of its scan range before the new thread announces.
    high_watermark.store(id + 1, std::memory_order_release);
    return id;
  }

  // Slot reuse safety: between the -1 store and a new thread's announce(),
  // try_advance() may see -1 and skip this slot.  This is correct — the old
  // thread has exited its critical section, and the new thread has not yet
  // entered one, so no reader holds a dangling pointer.  The new thread's
  // announce() retry loop guarantees it pins the current (or later) epoch.
  void release_id(int id) {
    slots[id].epoch.store(-1, std::memory_order_release);
    std::lock_guard<std::mutex> lk(pool_mutex);
    free_ids.push_back(id);
  }

  int announce(int id) {
    while (true) {
      long e = current.load(std::memory_order_acquire);
      slots[id].epoch.exchange(e, std::memory_order_seq_cst);
      if (current.load(std::memory_order_acquire) == e) return id;
    }
  }

  void unannounce(int id) {
    slots[id].epoch.store(-1, std::memory_order_release);
  }

  void try_advance() {
    long e = current.load(std::memory_order_acquire);
    int n = high_watermark.load(std::memory_order_acquire);
    for (int i = 0; i < n; i++) {
      long a = slots[i].epoch.load(std::memory_order_acquire);
      if (a != -1 && a < e) return;
    }
    current.compare_exchange_strong(e, e + 1, std::memory_order_release,
                                    std::memory_order_relaxed);
  }

  long get_current() const { return current.load(std::memory_order_acquire); }
};

// Leaking singleton avoids destruction-order issues with thread_local.
inline EpochState& state() {
  static EpochState* s = new EpochState();
  return *s;
}

// Per-thread registration with automatic ID recycling on thread exit.
struct ThreadReg {
  int id;
  int nest_count = 0;
  ThreadReg() : id(state().acquire_id()) {}
  ~ThreadReg() { state().release_id(id); }
};

inline ThreadReg& thread_reg() {
  thread_local ThreadReg reg;
  return reg;
}

// RAII guard.  Supports nesting (only the outermost announce/unannounce
// touches the global array) and move semantics (for iterators).
struct Guard {
  struct inactive_t {};
  static constexpr inactive_t inactive{};

  bool active_;

  Guard() : active_(true) {
    auto& reg = thread_reg();
    if (reg.nest_count++ == 0) state().announce(reg.id);
  }
  explicit Guard(inactive_t) : active_(false) {}

  ~Guard() {
    if (active_) {
      auto& reg = thread_reg();
      if (--reg.nest_count == 0) state().unannounce(reg.id);
    }
  }

  Guard(Guard&& o) noexcept : active_(o.active_) { o.active_ = false; }
  Guard& operator=(Guard&& o) noexcept {
    if (this != &o) {
      if (active_) {
        auto& reg = thread_reg();
        if (--reg.nest_count == 0) state().unannounce(reg.id);
      }
      active_ = o.active_;
      o.active_ = false;
    }
    return *this;
  }
  Guard(const Guard&) = delete;
  Guard& operator=(const Guard&) = delete;
};

}  // namespace ebr

// ---------------------------------------------------------------------------
// LockFreeOrderedHashMap
// ---------------------------------------------------------------------------

template <typename Key, typename Value>
class LockFreeOrderedHashMap {
 private:
  struct Node {
    std::pair<Key, Value> keyValue;
    base::Atomic<Node*> bucketNext;  // next in hash bucket chain
    base::Atomic<Node*> orderNext;   // next in insertion-order chain
    base::Atomic<bool> is_deleted;
    Node* staging_next;              // lock-free staging stack link
    bool bucket_swept;               // GC unlinked from bucket chain
    bool order_swept;                // GC unlinked from order chain
    long retired_epoch;              // epoch when fully unlinked (-1 = live)

    Node(const Key& k, Value&& v)
        : keyValue(std::make_pair(k, std::move(v))),
          bucketNext(nullptr),
          orderNext(nullptr),
          is_deleted(false),
          staging_next(nullptr),
          bucket_swept{false},
          order_swept(false),
          retired_epoch(-1) {}

    Node(Key&& k, Value&& v)
        : keyValue(std::make_pair(std::move(k), std::move(v))),
          bucketNext(nullptr),
          orderNext(nullptr),
          is_deleted(false),
          staging_next(nullptr),
          bucket_swept{false},
          order_swept(false),
          retired_epoch(-1) {}
  };

  base::Atomic<Node*>* buckets;
  mem_size bucketCount;
  std::hash<Key> keyHasher;

  base::Atomic<Node*> orderHead;
  base::Atomic<Node*> orderTail;

  // Lock-free Treiber stack: insert() pushes here via CAS (no mutex).
  base::Atomic<Node*> staging_head_{nullptr};

  // GC-owned state (only accessed under gc_mutex_).
  base::Mutex gc_mutex_;
  base::Vector<Node*> all_nodes_;

  // ---- internal helpers ----

  Node* find_in_bucket(const Key& key) const {
    mem_size index = hash_key(key);
    Node* curr = buckets[index].load(std::memory_order_acquire);
    while (curr) {
      if (curr->keyValue.first == key) return curr;
      curr = curr->bucketNext.load(std::memory_order_acquire);
    }
    return nullptr;
  }

  mem_size hash_key(const Key& key) const {
    if (bucketCount == 0)
      throw std::logic_error("Bucket count is zero");
    return keyHasher(key) % bucketCount;
  }

  // Re-traverse from the bucket head to find the correct predecessor, then
  // CAS-unlink.  Immune to stale-prev issues: the predecessor is always
  // fresh at the point of the CAS.
  bool try_unlink_from_bucket(Node* node) {
    mem_size index = hash_key(node->keyValue.first);
    base::Atomic<Node*>* prev_ptr = &buckets[index];
    Node* curr = prev_ptr->load(std::memory_order_acquire);
    while (curr) {
      if (curr == node) {
        Node* expected = node;
        Node* next = node->bucketNext.load(std::memory_order_relaxed);
        return prev_ptr->compare_exchange_strong(
            expected, next, std::memory_order_release,
            std::memory_order_relaxed);
      }
      prev_ptr = &curr->bucketNext;
      curr = curr->bucketNext.load(std::memory_order_acquire);
    }
    return false;  // already unlinked
  }

  void stage_node(Node* node) {
    Node* old_head = staging_head_.load(std::memory_order_relaxed);
    do {
      node->staging_next = old_head;
    } while (!staging_head_.compare_exchange_weak(
        old_head, node, std::memory_order_release,
        std::memory_order_relaxed));
  }

 public:
  // ---- Iterators (CRTP, no vtable, epoch-protected) ----
  // NOTE: Iterators hold an ebr::Guard for the lifetime of traversal.
  // A long-lived iterator pins the epoch and prevents *all* garbage
  // collection from making progress.  Avoid storing iterators across
  // operations; prefer short-lived traversals or ordered_keys_snapshot().

  template <typename IteratorType>
  class IteratorBase {
   protected:
    const LockFreeOrderedHashMap<Key, Value>* map;
    Node* currentNode;
    ebr::Guard guard_;

    void skip_deleted() {
      while (currentNode &&
             currentNode->is_deleted.load(std::memory_order_acquire)) {
        static_cast<IteratorType*>(this)->advance_impl();
      }
    }

   public:
    using KeyValuePair = std::pair<Key, Value>;

    // Active iterator: pins the epoch so traversed nodes stay alive.
    IteratorBase(const LockFreeOrderedHashMap<Key, Value>* m, Node* start_node)
        : map(m), currentNode(start_node), guard_() {
      skip_deleted();
    }

    // End sentinel: no epoch pin needed.
    IteratorBase(const LockFreeOrderedHashMap<Key, Value>* m)
        : map(m), currentNode(nullptr), guard_(ebr::Guard::inactive) {}

    KeyValuePair& operator*() const { return currentNode->keyValue; }
    KeyValuePair* operator->() const { return &currentNode->keyValue; }

    bool operator==(const IteratorBase& other) const {
      return currentNode == other.currentNode && map == other.map;
    }
    bool operator!=(const IteratorBase& other) const {
      return !(*this == other);
    }

    IteratorType& operator++() {
      if (currentNode) {
        static_cast<IteratorType*>(this)->advance_impl();
        skip_deleted();
      }
      return static_cast<IteratorType&>(*this);
    }
  };

  class OrderIterator : public IteratorBase<OrderIterator> {
    friend class IteratorBase<OrderIterator>;
    friend class LockFreeOrderedHashMap;

    void advance_impl() {
      if (this->currentNode)
        this->currentNode =
            this->currentNode->orderNext.load(std::memory_order_acquire);
    }

   public:
    OrderIterator(const LockFreeOrderedHashMap<Key, Value>* m,
                  Node* start_node)
        : IteratorBase<OrderIterator>(m, start_node) {}
    OrderIterator(const LockFreeOrderedHashMap<Key, Value>* m)
        : IteratorBase<OrderIterator>(m) {}
  };

  class BucketIterator : public IteratorBase<BucketIterator> {
    friend class IteratorBase<BucketIterator>;
    friend class LockFreeOrderedHashMap;

   private:
    mem_size bucketIndex;

    void advance_impl() {
      if (!this->currentNode) return;
      this->currentNode =
          this->currentNode->bucketNext.load(std::memory_order_acquire);
      while (!this->currentNode && bucketIndex < this->map->bucketCount - 1) {
        ++bucketIndex;
        this->currentNode =
            this->map->buckets[bucketIndex].load(std::memory_order_acquire);
      }
      if (!this->currentNode) bucketIndex = this->map->bucketCount;
    }

   public:
    BucketIterator(const LockFreeOrderedHashMap<Key, Value>* m,
                   mem_size b_idx,
                   Node* start_node)
        : IteratorBase<BucketIterator>(m, start_node), bucketIndex(b_idx) {}
    BucketIterator(const LockFreeOrderedHashMap<Key, Value>* m)
        : IteratorBase<BucketIterator>(m), bucketIndex(m->bucketCount) {}

    bool operator==(const BucketIterator& other) const {
      return this->currentNode == other.currentNode &&
             bucketIndex == other.bucketIndex && this->map == other.map;
    }
    bool operator!=(const BucketIterator& other) const {
      return !(*this == other);
    }
  };

  // ---- begin / end ----

  OrderIterator order_begin() const {
    return OrderIterator(this,
                         orderHead.load(std::memory_order_acquire));
  }
  OrderIterator order_end() const { return OrderIterator(this); }

  BucketIterator begin() const {
    for (mem_size i = 0; i < bucketCount; ++i) {
      Node* node = buckets[i].load(std::memory_order_acquire);
      while (node && node->is_deleted.load(std::memory_order_acquire))
        node = node->bucketNext.load(std::memory_order_acquire);
      if (node) return BucketIterator(this, i, node);
    }
    return end();
  }
  BucketIterator end() const { return BucketIterator(this); }

  // ---- Constructor / Destructor ----

  explicit LockFreeOrderedHashMap(mem_size count)
      : bucketCount(count > 0 ? count : 1),
        orderHead(nullptr),
        orderTail(nullptr) {
    buckets = new base::Atomic<Node*>[bucketCount];
    for (mem_size i = 0; i < bucketCount; ++i)
      buckets[i].store(nullptr, std::memory_order_relaxed);
  }

  // Destructor assumes no concurrent operations.  The staging stack and
  // all_nodes_ are disjoint: staging holds nodes pushed since the last GC,
  // while all_nodes_ holds nodes drained by previous GC passes.  Both sets
  // are walked and freed unconditionally (EBR safety is irrelevant during
  // single-threaded teardown).
  ~LockFreeOrderedHashMap() {
    Node* staged = staging_head_.load(std::memory_order_relaxed);
    while (staged) {
      Node* next = staged->staging_next;
      delete staged;
      staged = next;
    }
    for (Node* n : all_nodes_)
      delete n;
    delete[] buckets;
  }

  LockFreeOrderedHashMap(const LockFreeOrderedHashMap&) = delete;
  LockFreeOrderedHashMap& operator=(const LockFreeOrderedHashMap&) = delete;
  LockFreeOrderedHashMap(LockFreeOrderedHashMap&&) = delete;
  LockFreeOrderedHashMap& operator=(LockFreeOrderedHashMap&&) = delete;

  // ---- Core operations ----
  // NOTE: No rehashing path — performance degrades at high load factors.
  // Callers should size the bucket count for expected peak occupancy.

  bool insert(const Key& key, Value&& value) {
    return insert_internal(key, std::move(value));
  }
  bool insert(Key&& key, Value&& value) {
    return insert_internal(std::move(key), std::move(value));
  }

 private:
  template <typename K, typename V>
  bool insert_internal(K&& key, V&& value) {
    ebr::Guard guard;
    Node* newNode = nullptr;
    mem_size index = hash_key(key);

    // Phase 1: insert into hash bucket.
    while (true) {
      Node* existingNode = find_in_bucket(key);

      if (existingNode) {
        if (!existingNode->is_deleted.load(std::memory_order_acquire)) {
          delete newNode;
          return false;
        }
        // Deleted node still linked. Help unlink and retry.
        try_unlink_from_bucket(existingNode);
        continue;
      }

      if (!newNode)
        newNode = new Node(std::forward<K>(key), std::forward<V>(value));

      Node* oldHead = buckets[index].load(std::memory_order_acquire);
      newNode->bucketNext.store(oldHead, std::memory_order_relaxed);

      if (buckets[index].compare_exchange_weak(
              oldHead, newNode, std::memory_order_release,
              std::memory_order_relaxed)) {
        // Post-CAS duplicate detection.
        //
        // Correctness relies on the prepend-chain ordering invariant:
        // the scan follows newNode→bucketNext (toward *older* nodes),
        // so it only sees nodes that CAS'd *before* us (they are deeper
        // in the chain).  A node that CAS'd *after* us sits between the
        // bucket head and us — unreachable from bucketNext.
        //
        // Among any set of concurrent duplicates for the same key, exactly
        // one — the deepest (first to CAS) — finds no duplicate behind
        // itself and proceeds.  Every shallower node finds the deeper one
        // and backs off.  No symmetric tiebreaker is needed because the
        // acyclic singly-linked chain provides natural asymmetry.
        Node* check =
            newNode->bucketNext.load(std::memory_order_acquire);
        while (check) {
          if (check->keyValue.first == key &&
              !check->is_deleted.load(std::memory_order_acquire)) {
            // We are shallower; back off. The deeper node wins.
            newNode->is_deleted.store(true, std::memory_order_release);
            newNode->order_swept = true;  // never entered order chain
            try_unlink_from_bucket(newNode);
            stage_node(newNode);
            return false;
          }
          check = check->bucketNext.load(std::memory_order_acquire);
        }
        stage_node(newNode);
        break;
      }
    }

    // Phase 2: append to insertion-order list (Michael & Scott).
    newNode->orderNext.store(nullptr, std::memory_order_relaxed);
    Node* expected_tail = nullptr;

    while (true) {
      Node* current_tail = orderTail.load(std::memory_order_acquire);

      if (current_tail == nullptr) {
        if (orderHead.compare_exchange_weak(
                expected_tail, newNode, std::memory_order_release,
                std::memory_order_relaxed)) {
          orderTail.compare_exchange_strong(
              expected_tail, newNode, std::memory_order_release,
              std::memory_order_relaxed);
          return true;
        }
        expected_tail = nullptr;
        continue;
      }

      expected_tail = current_tail;
      Node* tail_next =
          current_tail->orderNext.load(std::memory_order_acquire);

      if (orderTail.load(std::memory_order_acquire) != current_tail)
        continue;

      if (tail_next != nullptr) {
        orderTail.compare_exchange_weak(
            current_tail, tail_next, std::memory_order_release,
            std::memory_order_relaxed);
        continue;
      }

      if (current_tail->orderNext.compare_exchange_weak(
              tail_next, newNode, std::memory_order_release,
              std::memory_order_relaxed)) {
        orderTail.compare_exchange_strong(
            current_tail, newNode, std::memory_order_release,
            std::memory_order_relaxed);
        return true;
      }
    }
  }

 public:
  // Walks the bucket chain looking for the first *live* matching node.
  //
  // It would be tempting to reuse find_in_bucket() here, but that helper
  // returns the first match by key regardless of is_deleted — which is
  // what insert/remove want (so they can help-unlink tagged-but-still-
  // linked nodes). For the public find() we need to skip past tagged
  // nodes: a concurrent remove may have marked an old node as deleted
  // but lost the unlink CAS to contention, leaving it linked at the head
  // of the bucket chain even though a *newer* live node for the same key
  // sits deeper. Returning the deleted shallow node and giving up would
  // miss the live one.
  bool find(const Key& key, Value& value) const {
    ebr::Guard guard;
    mem_size index = hash_key(key);
    Node* curr = buckets[index].load(std::memory_order_acquire);
    while (curr) {
      if (curr->keyValue.first == key &&
          !curr->is_deleted.load(std::memory_order_acquire)) {
        value = curr->keyValue.second;
        return true;
      }
      curr = curr->bucketNext.load(std::memory_order_acquire);
    }
    return false;
  }

  template <typename Callback>
  bool with_value(const Key& key, Callback&& callback) const {
    ebr::Guard guard;
    mem_size index = hash_key(key);
    Node* curr = buckets[index].load(std::memory_order_acquire);
    while (curr) {
      if (curr->keyValue.first == key &&
          !curr->is_deleted.load(std::memory_order_acquire)) {
        callback(curr->keyValue.second);
        return true;
      }
      curr = curr->bucketNext.load(std::memory_order_acquire);
    }
    return false;
  }

  // Epoch-based garbage collector.
  // 1. Drain staging stack.
  // 2. Sweep bucket chains (CAS-unlink deleted nodes).
  // 3. Sweep order chain  (unlink deleted non-tail nodes).
  // 4. Stamp the retirement epoch on fully-unlinked nodes.
  // 5. Advance the global epoch.
  // 6. Free nodes whose retirement epoch is 2+ epochs in the past
  //    (guaranteed unreachable by any concurrent reader).
  void collect_garbage() {
    std::lock_guard<base::Mutex> lock(gc_mutex_);
    collect_garbage_locked();
  }

  bool remove(const Key& key) {
    bool removed = remove_guarded(key);
    post_mutate();
    return removed;
  }

  // Snapshot of keys in insertion order (skips deleted entries).
  base::Vector<Key> ordered_keys_snapshot() const {
    ebr::Guard guard;
    base::Vector<Key> keys;
    Node* curr = orderHead.load(std::memory_order_acquire);
    while (curr) {
      if (!curr->is_deleted.load(std::memory_order_acquire))
        keys.push_back(curr->keyValue.first);
      curr = curr->orderNext.load(std::memory_order_acquire);
    }
    return keys;
  }

 private:
  // ---- Auto-GC infrastructure ----

  static constexpr mem_size kAutoGCThreshold = 4096;
  base::Atomic<mem_size> delete_since_gc_{0};
  base::Atomic<mem_size> op_count_{0};

  bool remove_guarded(const Key& key) {
    ebr::Guard guard;
    while (true) {
      Node* node = find_in_bucket(key);
      if (!node) return false;

      bool expected = false;
      if (node->is_deleted.compare_exchange_weak(
              expected, true, std::memory_order_release,
              std::memory_order_relaxed)) {
        try_unlink_from_bucket(node);
        delete_since_gc_.fetch_add(1, std::memory_order_relaxed);
        return true;
      }
      if (expected) return false;
    }
  }

  // Called after every insert/remove, outside the EBR guard scope, so the
  // epoch can advance freely and retired nodes can actually be freed.
  void post_mutate() {
    mem_size n = op_count_.fetch_add(1, std::memory_order_relaxed);
    // Periodically advance the epoch so retired nodes become freeable.
    if ((n & 0xFF) == 0)
      ebr::state().try_advance();
    // Trigger GC when enough deletions have accumulated.  Keyed off
    // delete count (not staging count) so insert-heavy workloads don't
    // pay for GC sweeps on live nodes.  try_to_lock avoids blocking the
    // hot path if GC is already running on another thread.
    if (delete_since_gc_.load(std::memory_order_relaxed) >=
        kAutoGCThreshold) {
      std::unique_lock<base::Mutex> lock(gc_mutex_, std::try_to_lock);
      if (lock.owns_lock())
        collect_garbage_locked();
    }
  }

  void collect_garbage_locked() {
    // 1. Drain lock-free staging stack into the tracked list.
    Node* staged =
        staging_head_.exchange(nullptr, std::memory_order_acquire);
    while (staged) {
      Node* next = staged->staging_next;
      all_nodes_.push_back(staged);
      staged = next;
    }
    delete_since_gc_.store(0, std::memory_order_relaxed);

    // 2. Sweep bucket chains: CAS-unlink deleted nodes.
    for (mem_size i = 0; i < bucketCount; ++i) {
      base::Atomic<Node*>* prev_ptr = &buckets[i];
      Node* curr = prev_ptr->load(std::memory_order_acquire);
      while (curr) {
        Node* next = curr->bucketNext.load(std::memory_order_acquire);
        if (curr->is_deleted.load(std::memory_order_acquire)) {
          Node* expected = curr;
          if (prev_ptr->compare_exchange_strong(
                  expected, next, std::memory_order_release,
                  std::memory_order_relaxed)) {
            curr->bucket_swept = true;
            curr = next;
            continue;
          }
          prev_ptr = &buckets[i];
          curr = prev_ptr->load(std::memory_order_acquire);
          continue;
        }
        prev_ptr = &curr->bucketNext;
        curr = next;
      }
    }

    // 3. CAS-unlink deleted non-tail nodes from the order chain.
    Node* prev = nullptr;
    Node* curr = orderHead.load(std::memory_order_acquire);
    while (curr) {
      Node* next = curr->orderNext.load(std::memory_order_acquire);
      if (curr->is_deleted.load(std::memory_order_acquire) &&
          next != nullptr) {
        Node* expected = curr;
        bool unlinked;
        if (prev) {
          unlinked = prev->orderNext.compare_exchange_strong(
              expected, next, std::memory_order_release,
              std::memory_order_relaxed);
        } else {
          unlinked = orderHead.compare_exchange_strong(
              expected, next, std::memory_order_release,
              std::memory_order_relaxed);
        }
        if (unlinked) {
          curr->order_swept = true;
        } else {
          if (prev)
            curr = prev->orderNext.load(std::memory_order_acquire);
          else
            curr = orderHead.load(std::memory_order_acquire);
          continue;
        }
      } else {
        prev = curr;
      }
      curr = next;
    }

    // 4. Stamp retirement epoch on fully-swept nodes.
    long current_e = ebr::state().get_current();
    for (mem_size i = 0; i < all_nodes_.size(); ++i) {
      Node* n = all_nodes_[i];
      if (n->bucket_swept &&
          n->order_swept && n->retired_epoch < 0)
        n->retired_epoch = current_e;
    }

    // 5. Advance the global epoch.
    ebr::state().try_advance();

    // 6. Free nodes retired 2+ epochs ago (provably unreachable).
    long safe = ebr::state().get_current();
    mem_size write_idx = 0;
    for (mem_size i = 0; i < all_nodes_.size(); ++i) {
      if (all_nodes_[i]->retired_epoch >= 0 &&
          all_nodes_[i]->retired_epoch + 2 <= safe) {
        delete all_nodes_[i];
      } else {
        all_nodes_[write_idx++] = all_nodes_[i];
      }
    }
    all_nodes_.resize(write_idx);
  }
};

}  // namespace base

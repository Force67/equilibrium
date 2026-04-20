#pragma once
// c++ translation of
// https://github.com/dbittman/waitfree-mpsc-queue/blob/master/mpsc.c

#include <new>
#include <base/arch.h>
#include <base/atomic.h>
#include <base/memory/move.h>

namespace base {

// Wait-free MPSC linked-list queue.
//
// The queue keeps a sentinel node ahead of the data nodes. Historically the
// sentinel embedded a default-constructed `T value;`, which made the whole
// container unusable for non-default-constructible payloads. The Node now
// stores `value` inside an anonymous union so its lifetime is managed by hand:
// the sentinel never constructs `value`; data nodes do; dequeue destroys the
// value after copying it out; the destructor walks the chain destroying live
// payloads.
template <typename T>
class MPSCQueue {
 public:
  struct Node {
    // Anonymous union: `value` storage exists but is NOT constructed by
    // default. The queue is responsible for constructing/destroying it.
    union {
      T value;
    };
    base::Atomic<Node*> next;

    // Sentinel constructor — leaves `value` uninitialized.
    Node() : next(nullptr) {}

    // Data-node constructor — emplaces value in-place.
    template <typename... Args>
    explicit Node(Args&&... args) : next(nullptr) {
      ::new (static_cast<void*>(&value)) T(base::forward<Args>(args)...);
    }

    // Destructor is a no-op; the queue destroys `value` explicitly before
    // freeing the node so we don't double-destroy or touch a sentinel that
    // never had a value constructed.
    ~Node() {}

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
  };

  // Iteration only ever visits data nodes (begin() skips the sentinel), so
  // dereferencing `node_->value` is always defined.
  class Iterator {
   public:
    explicit Iterator(Node* node) : node_(node) {}

    T& operator*() const { return node_->value; }
    T* operator->() const { return &node_->value; }

    Iterator& operator++() {
      node_ = node_->next.load(base::memory_order_acquire);
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const Iterator& other) const { return node_ == other.node_; }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    Node* node_;
  };

  Iterator begin() const {
    Node* tail = tail_.load(base::memory_order_acquire);
    return Iterator(tail->next.load(base::memory_order_acquire));
  }
  Iterator end() const { return Iterator(nullptr); }

  MPSCQueue() : head_(new Node), tail_(head_.load(base::memory_order_relaxed)) {}

  ~MPSCQueue() {
    // Sentinel first (no value to destroy), then walk the rest of the chain
    // destroying any live payloads before freeing.
    Node* node = tail_.load(base::memory_order_relaxed);
    Node* next = node->next.load(base::memory_order_relaxed);
    delete node;  // sentinel — value never constructed
    while (next) {
      Node* after = next->next.load(base::memory_order_relaxed);
      next->value.~T();
      delete next;
      next = after;
    }
  }

  MPSCQueue(const MPSCQueue&) = delete;
  MPSCQueue& operator=(const MPSCQueue&) = delete;

  void enqueue(T&& value) {
    Node* node = new Node(base::move(value));
    Node* prev_head = head_.exchange(node, base::memory_order_acq_rel);
    prev_head->next.store(node, base::memory_order_release);
  }

  void enqueue(const T& value) {
    Node* node = new Node(value);
    Node* prev_head = head_.exchange(node, base::memory_order_acq_rel);
    prev_head->next.store(node, base::memory_order_release);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    Node* node = new Node(base::forward<Args>(args)...);
    Node* prev_head = head_.exchange(node, base::memory_order_acq_rel);
    prev_head->next.store(node, base::memory_order_release);
  }

  bool dequeue(T& out) {
    Node* tail = tail_.load(base::memory_order_relaxed);
    Node* next = tail->next.load(base::memory_order_acquire);
    if (next == nullptr) return false;

    // Move the value out, then destroy the source. After this point `next`
    // becomes the new sentinel and must NOT have a live `value` (otherwise the
    // next dequeue / destruction would touch a moved-from / wrong value).
    out = base::move(next->value);
    next->value.~T();

    tail_.store(next, base::memory_order_release);
    delete tail;  // old sentinel — no value to destroy
    return true;
  }

  // Caller borrows the front element. The pointer is valid until the next
  // dequeue / removeFront. Returns false if the queue is empty.
  bool peek(T*& value) const {
    Node* tail = tail_.load(base::memory_order_relaxed);
    Node* next = tail->next.load(base::memory_order_acquire);
    if (next == nullptr) return false;
    value = &next->value;
    return true;
  }

  bool removeFront() {
    Node* tail = tail_.load(base::memory_order_relaxed);
    Node* next = tail->next.load(base::memory_order_acquire);
    if (next == nullptr) return false;

    next->value.~T();
    tail_.store(next, base::memory_order_release);
    delete tail;
    return true;
  }

  bool empty() const {
    return tail_.load(base::memory_order_acquire)
               ->next.load(base::memory_order_acquire) == nullptr;
  }

  // Approximate queue size. O(n) walk -- use sparingly.
  mem_size size_approx() const {
    mem_size count = 0;
    Node* current = tail_.load(base::memory_order_acquire)
                        ->next.load(base::memory_order_acquire);
    while (current != nullptr) {
      ++count;
      current = current->next.load(base::memory_order_acquire);
    }
    return count;
  }

 private:
  base::Atomic<Node*> head_;
  base::Atomic<Node*> tail_;
};

}  // namespace base

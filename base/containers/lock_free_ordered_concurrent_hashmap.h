// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <base/atomic.h>
#include <base/memory/move.h>

namespace base {
template <typename Key, typename Value>
class OrderedLockFreeHashMap {
 public:
  struct Node {
    std::pair<Key, Value> keyValue;
    std::atomic<Node*> next;
    // orderNext / orderPrev are atomic so concurrent inserts can splice
    // nodes in without data races. The list itself is still single-writer-
    // friendly (concurrent erase races with inserts), but pure concurrent
    // inserts followed by reads are now well-defined.
    std::atomic<Node*> orderNext;
    std::atomic<Node*> orderPrev;

    Node(const Key& k, Value&& v)
        : keyValue(std::make_pair(k, std::move(v))),
          next(nullptr),
          orderNext(nullptr),
          orderPrev(nullptr) {}
  };

  class Iterator {
   private:
    const OrderedLockFreeHashMap<Key, Value>* map;
    size_t bucketIndex;
    Node* currentNode;

   public:
    using KeyValuePair = std::pair<Key, Value>;

    Iterator(const OrderedLockFreeHashMap<Key, Value>* map,
             size_t bucketIndex,
             Node* node)
        : map(map), bucketIndex(bucketIndex), currentNode(node) {}

    Iterator& operator++() {
      if (currentNode) {
        currentNode = currentNode->next.load(std::memory_order_acquire);
      }
      while (!currentNode && bucketIndex < map->bucketCount - 1) {
        ++bucketIndex;
        currentNode = map->buckets[bucketIndex].load(std::memory_order_acquire);
      }
      return *this;
    }

    KeyValuePair& operator*() { return currentNode->keyValue; }
    KeyValuePair* operator->() { return &currentNode->keyValue; }

    // Equality and inequality operators
    bool operator==(const Iterator& other) const {
      return currentNode == other.currentNode;
    }
    bool operator!=(const Iterator& other) const { return !(*this == other); }
  };

  Iterator begin() {
    for (size_t i = 0; i < bucketCount; ++i) {
      Node* node = buckets[i].load();
      if (node) {
        return Iterator(this, i, node);
      }
    }
    return end();
  }

  Iterator end() { return Iterator(this, bucketCount, nullptr); }

  OrderedLockFreeHashMap(size_t count)
      : bucketCount(count), orderHead(nullptr), orderTail(nullptr) {
    buckets = new std::atomic<Node*>[count];
    for (size_t i = 0; i < count; ++i) {
      buckets[i].store(nullptr);
    }
  }

  ~OrderedLockFreeHashMap() {
    // Delete all nodes in each bucket
    for (size_t i = 0; i < bucketCount; ++i) {
      Node* head = buckets[i].load();
      while (head != nullptr) {
        Node* next = head->next.load();
        delete head;
        head = next;
      }
    }

    // Delete the buckets array
    delete[] buckets;

    // Since all nodes are already deleted, and each node is part of the ordered
    // list, there's no need to traverse the ordered list again for deletion.
    // However, if additional resources are allocated for the ordered list
    // management, they should be freed here.
  }

  void insert(const Key& key, Value&& value) {
    Node* newNode = new Node(key, std::move(value));

    size_t index = hash(key);
    Node* oldHead = buckets[index].load(std::memory_order_acquire);

    // Insertion into the bucket list (Treiber-style head push).
    do {
      newNode->next.store(oldHead, std::memory_order_relaxed);
    } while (!buckets[index].compare_exchange_weak(
        oldHead, newNode, std::memory_order_release, std::memory_order_acquire));

    // Insertion into the ordered list:
    //   1. CAS orderTail from oldTail → newNode. Whichever thread wins owns
    //      the right to publish the back-link `oldTail->orderNext = newNode`.
    //   2. After the CAS succeeds, publish the back-link. Each `oldTail`
    //      value is consumed by exactly one CAS winner, so this write has a
    //      single writer and is race-free.
    //
    // The newNode->orderPrev write happens before the CAS, while newNode is
    // still private to this thread (no other thread can reach it yet).
    //
    // Concurrent ordered-list READS are still considered unsafe with
    // concurrent inserts because a reader could observe a tail node whose
    // orderNext hasn't been published yet (it would see a "short" list).
    // After all writers join, the list is fully linked.
    Node* oldTail;
    for (;;) {
      oldTail = orderTail.load(std::memory_order_acquire);
      newNode->orderPrev.store(oldTail, std::memory_order_relaxed);
      if (orderTail.compare_exchange_weak(
              oldTail, newNode,
              std::memory_order_release,
              std::memory_order_relaxed)) {
        break;
      }
    }
    if (oldTail) {
      oldTail->orderNext.store(newNode, std::memory_order_release);
    } else {
      orderHead.store(newNode, std::memory_order_release);
    }
  }

  bool find(const Key& key, Value& value) {
    size_t index = hash(key);
    Node* head = buckets[index].load(std::memory_order_acquire);

    while (head) {
      if (head->keyValue.first == key) {
        value = head->keyValue.second;
        return true;
      }
      head = head->next.load(std::memory_order_acquire);
    }

    return false;
  }

  // WARNING: remove() is NOT safe to call concurrently with find() or
  // iteration. See LockFreeHashMap::remove() for details.
  bool remove(const Key& key) {
    size_t index = hash(key);
    Node* current = buckets[index].load(std::memory_order_acquire);
    Node* prev = nullptr;

    // Find and remove from the bucket list
    while (current) {
      if (current->keyValue.first == key) {
        Node* next = current->next.load(std::memory_order_acquire);
        if (prev) {
          prev->next.store(next, std::memory_order_release);
        } else if (!buckets[index].compare_exchange_strong(
                       current, next, std::memory_order_acq_rel)) {
          continue;
        }

        // Remove from the ordered list. This is single-writer territory:
        // erase races with other inserts/erases are still unsafe (would need
        // hazard pointers / RCU). The atomic loads/stores on orderNext/Prev
        // are required for the C++ memory model even in the single-writer
        // case to avoid data races against the atomic writes inside insert().
        Node* prev_o = current->orderPrev.load(std::memory_order_acquire);
        Node* next_o = current->orderNext.load(std::memory_order_acquire);
        if (prev_o) {
          prev_o->orderNext.store(next_o, std::memory_order_release);
        } else {
          orderHead.store(next_o, std::memory_order_release);
        }
        if (next_o) {
          next_o->orderPrev.store(prev_o, std::memory_order_release);
        } else {
          orderTail.store(prev_o, std::memory_order_release);
        }

        delete current;
        return true;
      }
      prev = current;
      current = current->next.load(std::memory_order_acquire);
    }

    return false;
  }

  // Iterator class and other methods...

 public:
  std::atomic<Node*>* buckets;
  size_t bucketCount;

  std::atomic<Node*> orderHead;
  std::atomic<Node*> orderTail;

  size_t hash(const Key& key) const { return key % bucketCount; }

  // Other private methods...
};
}  // namespace base
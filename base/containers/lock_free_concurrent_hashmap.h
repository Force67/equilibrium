// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstddef>

#include <base/atomic.h>
#include <base/containers/pair.h>
#include <base/memory/move.h>

namespace base {
// LockFreeHashMap is a concurrent hash map implementation that provides
// lock-free
// operations for insertion, deletion, and lookup. It's designed to handle
// high-concurrency scenarios efficiently by minimizing blocking and
// synchronization.
//
// How it Works:
// - The map consists of a fixed number of buckets, with each bucket containing
// a
//   singly linked list of nodes.
// - Each node stores a key-value pair, similar to a standard hash map.
// - Insertion, deletion, and lookup operations are performed using atomic
// operations,
//   ensuring that the map can be safely used by multiple threads without
//   explicit locks.
// - The hash function determines the bucket index for each key, and the node is
// then
//   inserted into the corresponding bucket's linked list.
//
// Ordering:
// - This implementation does not maintain the order of insertion. The elements
// in each
//   bucket follow the order in which they were inserted, but this order is not
//   preserved across the entire map.
// - The iteration order will follow the sequence of buckets and then the linked
// list within
//   each bucket, but this is not indicative of insertion order.
// - Due to its concurrent nature and bucket-based storage, the insertion order
// is not
//   deterministic, especially under high-concurrency scenarios.
//
// Note:
// - This class is suitable for scenarios where concurrent access to a hash map
// is required
//   and the order of elements is not a concern.
// - It provides efficient key-based lookup and modification operations with
// minimized
//   contention among threads.
template <typename Key, typename Value>
class LockFreeHashMap {
 public:
  using value_type = Value;
  using key_type = Key;

  struct Node;

  class Iterator {
   private:
    const LockFreeHashMap<Key, Value>* map;
    size_t bucketIndex;
    Node* currentNode;

   public:
    using KeyValuePair = base::Pair<Key, Value>;

    Iterator(const LockFreeHashMap<Key, Value>* map, size_t bucketIndex, Node* node)
        : map(map), bucketIndex(bucketIndex), currentNode(node) {
      AdvanceToLive();
    }

    Iterator& operator++() {
      if (currentNode) {
        currentNode = currentNode->next.load(base::memory_order_acquire);
      }
      AdvanceToLive();
      return *this;
    }

    KeyValuePair& operator*() { return currentNode->keyValue; }
    KeyValuePair* operator->() { return &currentNode->keyValue; }

    // Equality and inequality operators
    bool operator==(const Iterator& other) const {
      return currentNode == other.currentNode;
    }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    // Walks forward past tombstones and empty buckets.
    void AdvanceToLive() {
      for (;;) {
        while (currentNode &&
               currentNode->dead.load(base::memory_order_acquire)) {
          currentNode = currentNode->next.load(base::memory_order_acquire);
        }
        if (currentNode || bucketIndex >= map->bucketCount - 1) return;
        ++bucketIndex;
        currentNode = map->buckets[bucketIndex].load(base::memory_order_acquire);
      }
    }
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

  struct Node {
    base::Pair<Key, Value> keyValue;
    base::Atomic<Node*> next;
    // Removal is logical: a dead node stays linked so concurrent readers
    // can keep traversing it, and is only freed once the caller guarantees
    // quiescence (collect_garbage or the destructor).
    base::Atomic<bool> dead;

    Node(Key k, Value&& v)
        : keyValue{k, base::move(v)}, next(nullptr), dead(false) {}
  };

 private:
  base::Atomic<Node*>* buckets;
  size_t bucketCount;

  size_t hash(Key key) const { return key % bucketCount; }

 public:
  LockFreeHashMap(size_t count) : bucketCount(count) {
    buckets = new base::Atomic<Node*>[count];
    for (size_t i = 0; i < count; ++i) {
      buckets[i].store(nullptr);
    }
  }

  void insert(Key key, Value value) {
    size_t index = hash(key);
    Node* newNode = new Node(key, base::move(value));
    Node* oldHead = buckets[index].load(base::memory_order_acquire);

    do {
      newNode->next.store(oldHead, base::memory_order_relaxed);
    } while (!buckets[index].compare_exchange_weak(
        oldHead, newNode, base::memory_order_release, base::memory_order_acquire));
  }

  bool find(Key key, Value& value) const {
    size_t index = hash(key);
    Node* head = buckets[index].load(base::memory_order_acquire);

    while (head) {
      if (head->keyValue.first == key &&
          !head->dead.load(base::memory_order_acquire)) {
        value = head->keyValue.second;
        return true;
      }
      head = head->next.load(base::memory_order_acquire);
    }

    return false;
  }

  // Tombstones the first live node matching `key`. The node stays linked so
  // concurrent finds, removes and iteration never touch freed memory; the
  // bytes come back in collect_garbage() or the destructor.
  bool remove(Key key) {
    size_t index = hash(key);
    Node* head = buckets[index].load(base::memory_order_acquire);

    while (head) {
      if (head->keyValue.first == key &&
          !head->dead.load(base::memory_order_acquire)) {
        bool expected = false;
        if (head->dead.compare_exchange_strong(expected, true,
                                               base::memory_order_acq_rel)) {
          return true;
        }
        // Another thread tombstoned this node first; the key is gone.
        return false;
      }
      head = head->next.load(base::memory_order_acquire);
    }

    return false;
  }

  // Unlinks and frees tombstoned nodes. The caller must guarantee no
  // concurrent access of any kind for the duration (maintenance windows,
  // frame boundaries); this is the only place besides the destructor that
  // frees nodes.
  void collect_garbage() {
    for (size_t i = 0; i < bucketCount; ++i) {
      Node* node = buckets[i].load(base::memory_order_relaxed);
      Node* prev = nullptr;
      while (node) {
        Node* next = node->next.load(base::memory_order_relaxed);
        if (node->dead.load(base::memory_order_relaxed)) {
          if (prev) {
            prev->next.store(next, base::memory_order_relaxed);
          } else {
            buckets[i].store(next, base::memory_order_relaxed);
          }
          delete node;
        } else {
          prev = node;
        }
        node = next;
      }
    }
  }

  ~LockFreeHashMap() {
    for (size_t i = 0; i < bucketCount; ++i) {
      Node* head = buckets[i].load(base::memory_order_relaxed);
      while (head) {
        Node* next = head->next.load(base::memory_order_relaxed);
        delete head;
        head = next;
      }
    }
    delete[] buckets;
  }
};
}  // namespace base
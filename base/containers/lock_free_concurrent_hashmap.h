// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <base/atomic.h>
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
    using KeyValuePair = std::pair<Key, Value>;

    Iterator(const LockFreeHashMap<Key, Value>* map,
             size_t bucketIndex,
             Node* node)
        : map(map), bucketIndex(bucketIndex), currentNode(node) {}

    Iterator& operator++() {
      if (currentNode) {
        currentNode = currentNode->next.load();
      }
      while (!currentNode && bucketIndex < map->bucketCount - 1) {
        ++bucketIndex;
        currentNode = map->buckets[bucketIndex].load();
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

  struct Node {
    std::pair<Key, Value> keyValue;
    base::Atomic<Node*> next;

    Node(Key k, Value&& v)
        : keyValue(std::make_pair(k, base::move(v))), next(nullptr) {}
  };
 private:
  base::Atomic<Node*>* buckets;
  size_t bucketCount;

  size_t hash(Key key) const { return key % bucketCount; }

 public:
  LockFreeHashMap(size_t count) : bucketCount(count) {
    buckets = new std::atomic<Node*>[count];
    for (size_t i = 0; i < count; ++i) {
      buckets[i].store(nullptr);
    }
  }

  void insert(Key key, Value value) {
    size_t index = hash(key);
    Node* newNode = new Node(key, base::move(value));
    Node* oldHead = buckets[index].load();

    do {
      newNode->next = oldHead;
    } while (!buckets[index].compare_exchange_weak(oldHead, newNode));
  }

  bool find(Key key, Value& value) const {
    size_t index = hash(key);
    Node* head = buckets[index].load();

    while (head) {
      if (head->keyValue.first == key) {
        value = head->keyValue.second;
        return true;
      }
      head = head->next;
    }

    return false;
  }

  bool remove(Key key) {
    size_t index = hash(key);
    Node* head = buckets[index].load();
    Node* prev = nullptr;

    while (head) {
      if (head->keyValue.first == key) {
        Node* next = head->next;
        if (prev) {
          prev->next = next;
        } else if (!buckets[index].compare_exchange_strong(head, next)) {
          continue;
        }
        delete head;
        return true;
      }
      prev = head;
      head = head->next;
    }

    return false;
  }

  ~LockFreeHashMap() {
    for (size_t i = 0; i < bucketCount; ++i) {
      Node* head = buckets[i].load();
      while (head) {
        Node* next = head->next;
        delete head;
        head = next;
      }
    }
    delete[] buckets;
  }
};
}  // namespace base
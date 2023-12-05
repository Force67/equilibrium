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
    Node* orderNext;
    Node* orderPrev;

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
    Node* oldHead = buckets[index].load();

    // Insertion into the bucket list
    do {
      newNode->next = oldHead;
    } while (!buckets[index].compare_exchange_weak(oldHead, newNode));

    // Insertion into the ordered list
    Node* oldTail = orderTail.load();
    do {
      newNode->orderPrev = oldTail;
      if (oldTail) {
        oldTail->orderNext = newNode;
      } else {
        orderHead.store(newNode);
      }
    } while (!orderTail.compare_exchange_weak(oldTail, newNode));
  }

  bool find(const Key& key, Value& value) {
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

  bool remove(const Key& key) {
    size_t index = hash(key);
    Node* current = buckets[index].load();
    Node* prev = nullptr;

    // Find and remove from the bucket list
    while (current) {
      if (current->keyValue.first == key) {
        Node* next = current->next.load();
        if (prev) {
          prev->next = next;
        } else if (!buckets[index].compare_exchange_strong(current, next)) {
          continue;
        }

        // Remove from the ordered list
        if (current->orderPrev) {
          current->orderPrev->orderNext = current->orderNext;
        } else {
          orderHead.store(current->orderNext);
        }

        if (current->orderNext) {
          current->orderNext->orderPrev = current->orderPrev;
        } else {
          orderTail.store(current->orderPrev);
        }

        delete current;
        return true;
      }
      prev = current;
      current = current->next;
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
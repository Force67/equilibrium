// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <base/atomic.h>
#include <base/memory/move.h>

namespace base {
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

 private:
  struct Node {
    std::pair<Key, Value> keyValue;
    base::Atomic<Node*> next;

    Node(Key k, Value &&v) : keyValue(std::make_pair(k, base::move(v))), next(nullptr) {}
  };

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
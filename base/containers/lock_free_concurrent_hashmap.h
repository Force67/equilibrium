#pragma once

#include <base/atomic.h>

namespace base {
template <typename Key, typename Value>
class LockFreeHashMap {
 private:
  struct Node {
    Key key;
    Value value;
    base::Atomic<Node*> next;

    Node(Key k, Value v) : key(k), value(v), next(nullptr) {}
  };

  base::Atomic<Node*>* buckets;
  size_t bucketCount;

  size_t hash(Key key) const { return key % bucketCount; }

 public:
  using value_type = Value;
  using key_type = Key;

  LockFreeHashMap(size_t count) : bucketCount(count) {
    buckets = new std::atomic<Node*>[count];
    for (size_t i = 0; i < count; ++i) {
      buckets[i].store(nullptr);
    }
  }

  void insert(Key key, Value value) {
    size_t index = hash(key);
    Node* newNode = new Node(key, value);
    Node* oldHead = buckets[index].load();

    do {
      newNode->next = oldHead;
    } while (!buckets[index].compare_exchange_weak(oldHead, newNode));
  }

  bool find(Key key, Value& value) const {
    size_t index = hash(key);
    Node* head = buckets[index].load();

    while (head) {
      if (head->key == key) {
        value = head->value;
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
      if (head->key == key) {
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
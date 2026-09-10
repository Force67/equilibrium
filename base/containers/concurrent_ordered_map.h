// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Hash map that preserves insertion order, safe for concurrent use. Backed by
// a single base::SharedMutex:
//   - find() / iteration take a shared lock, so reads scale.
//   - insert() / remove() / clear() take an exclusive lock, so writes
//     serialize against each other and against readers.
//
// Deliberately not lock-free: a lock-free version needs hazard pointers or
// epoch-based reclamation to free nodes safely under concurrent remove. A
// shared lock is a small predictable cost on the read path instead.
//
// Layout: open-addressing buckets of Node* heads, plus a doubly-linked
// insertion-order list. Each node lives in exactly one bucket and one order
// position; both are spliced under the lock.
//
// Iterate with for_each_in_order() / for_each_in_order_mut(). Do not store
// raw Node* outside the callback; the lock is released on return.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/containers/pair.h>
#include <base/hashing/hash.h>
#include <base/memory/move.h>
#include <base/threading/lock_guard.h>
#include <base/threading/mutex.h>

namespace base {

template <typename Key, typename Value>
class ConcurrentOrderedMap {
 public:
  struct Node {
    base::Pair<Key, Value> keyValue;
    Node* next;       // next in bucket linked list
    Node* orderNext;  // next in insertion order
    Node* orderPrev;

    Node(const Key& k, Value&& v)
        : keyValue{k, base::move(v)},
          next(nullptr),
          orderNext(nullptr),
          orderPrev(nullptr) {}
    Node(const Key& k, const Value& v)
        : keyValue{k, v}, next(nullptr), orderNext(nullptr), orderPrev(nullptr) {}
  };

  explicit ConcurrentOrderedMap(mem_size bucketCount) : bucketCount_(bucketCount) {
    BASE_BUGCHECK(bucketCount > 0, "ConcurrentOrderedMap needs > 0 buckets");
    buckets_ = new Node*[bucketCount_];
    for (mem_size i = 0; i < bucketCount_; ++i)
      buckets_[i] = nullptr;
  }

  ~ConcurrentOrderedMap() {
    // Walk the order list, guaranteed to visit every live node exactly
    // once regardless of how the buckets are arranged.
    Node* n = orderHead_;
    while (n) {
      Node* next = n->orderNext;
      delete n;
      n = next;
    }
    delete[] buckets_;
  }

  ConcurrentOrderedMap(const ConcurrentOrderedMap&) = delete;
  ConcurrentOrderedMap& operator=(const ConcurrentOrderedMap&) = delete;

  // -- Writes ----------------------------------------------------------------

  // Insert a new entry. Like the original API, this does NOT check for
  // duplicates: pushing the same key twice yields two distinct entries
  // and find() returns whichever is at the head of the bucket list (the
  // most recent insert).  Use insert_or_assign() if you need overwrite.
  void insert(const Key& key, Value value) {
    Node* node = new Node(key, base::move(value));
    base::LockGuard<base::SharedMutex> lk(mutex_);
    BucketPushFront(node);
    OrderListAppend(node);
    ++size_;
  }

  // Insert if absent, otherwise overwrite. Returns true if a new entry
  // was created, false if an existing entry was overwritten.
  bool insert_or_assign(const Key& key, Value value) {
    base::LockGuard<base::SharedMutex> lk(mutex_);
    const mem_size idx = BucketIndex(key);
    for (Node* n = buckets_[idx]; n; n = n->next) {
      if (n->keyValue.first == key) {
        n->keyValue.second = base::move(value);
        return false;
      }
    }
    Node* node = new Node(key, base::move(value));
    node->next = buckets_[idx];
    buckets_[idx] = node;
    OrderListAppend(node);
    ++size_;
    return true;
  }

  // Remove the first matching entry. Returns true if a key was found.
  bool remove(const Key& key) {
    base::LockGuard<base::SharedMutex> lk(mutex_);
    const mem_size idx = BucketIndex(key);
    Node* prev = nullptr;
    Node* current = buckets_[idx];
    while (current) {
      if (current->keyValue.first == key) {
        if (prev) {
          prev->next = current->next;
        } else {
          buckets_[idx] = current->next;
        }
        OrderListUnlink(current);
        delete current;
        --size_;
        return true;
      }
      prev = current;
      current = current->next;
    }
    return false;
  }

  void clear() {
    base::LockGuard<base::SharedMutex> lk(mutex_);
    Node* n = orderHead_;
    while (n) {
      Node* next = n->orderNext;
      delete n;
      n = next;
    }
    for (mem_size i = 0; i < bucketCount_; ++i)
      buckets_[i] = nullptr;
    orderHead_ = nullptr;
    orderTail_ = nullptr;
    size_ = 0;
  }

  // -- Reads -----------------------------------------------------------------

  bool find(const Key& key, Value& outValue) const {
    base::SharedLockGuard<base::SharedMutex> lk(mutex_);
    const mem_size idx = BucketIndex(key);
    for (Node* n = buckets_[idx]; n; n = n->next) {
      if (n->keyValue.first == key) {
        outValue = n->keyValue.second;
        return true;
      }
    }
    return false;
  }

  bool contains(const Key& key) const {
    base::SharedLockGuard<base::SharedMutex> lk(mutex_);
    const mem_size idx = BucketIndex(key);
    for (Node* n = buckets_[idx]; n; n = n->next) {
      if (n->keyValue.first == key)
        return true;
    }
    return false;
  }

  mem_size size() const {
    base::SharedLockGuard<base::SharedMutex> lk(mutex_);
    return size_;
  }

  bool empty() const {
    base::SharedLockGuard<base::SharedMutex> lk(mutex_);
    return size_ == 0;
  }

  // Snapshot iteration in insertion order. The functor runs while the
  // shared lock is held, don't perform any operation on the map from
  // within it (you'd self-deadlock when a writer is waiting).
  template <typename Func>
  void for_each_in_order(Func f) const {
    base::SharedLockGuard<base::SharedMutex> lk(mutex_);
    for (Node* n = orderHead_; n; n = n->orderNext) {
      f(n->keyValue.first, n->keyValue.second);
    }
  }

  // Mutating iteration. Holds the exclusive lock, same self-call caveat.
  template <typename Func>
  void for_each_in_order_mut(Func f) {
    base::LockGuard<base::SharedMutex> lk(mutex_);
    for (Node* n = orderHead_; n; n = n->orderNext) {
      f(n->keyValue.first, n->keyValue.second);
    }
  }

 private:
  mem_size BucketIndex(const Key& key) const {
    return base::Hash<Key>{}(key) % bucketCount_;
  }

  void BucketPushFront(Node* node) {
    const mem_size idx = BucketIndex(node->keyValue.first);
    node->next = buckets_[idx];
    buckets_[idx] = node;
  }

  void OrderListAppend(Node* node) {
    node->orderPrev = orderTail_;
    node->orderNext = nullptr;
    if (orderTail_) {
      orderTail_->orderNext = node;
    } else {
      orderHead_ = node;
    }
    orderTail_ = node;
  }

  void OrderListUnlink(Node* node) {
    if (node->orderPrev) {
      node->orderPrev->orderNext = node->orderNext;
    } else {
      orderHead_ = node->orderNext;
    }
    if (node->orderNext) {
      node->orderNext->orderPrev = node->orderPrev;
    } else {
      orderTail_ = node->orderPrev;
    }
  }

  mutable base::SharedMutex mutex_;
  Node** buckets_ = nullptr;
  mem_size bucketCount_ = 0;
  Node* orderHead_ = nullptr;
  Node* orderTail_ = nullptr;
  mem_size size_ = 0;
};

}  // namespace base

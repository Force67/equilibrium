// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::StableMap — pointer-stable hash map.
//
// Wraps UnorderedMap<K, V*> so that values are heap-allocated and their
// addresses never change on rehash.  Provides the same API surface as
// UnorderedMap<K, V>: operator[] returns V&, find() returns V*, and
// range-for yields (K& key, V& value).
//
// Use this instead of UnorderedMap when pointers/references to values must
// remain valid across insertions (e.g. values shared with worker threads,
// stored in task queues, or held across frame boundaries).
#pragma once

#include <base/containers/unordered_map.h>
#include <base/memory/move.h>

namespace base {

template <typename K, typename V, class THash = Hash<K>,
          class TEqual = Equal<K>, class TAllocator = DefaultAllocator>
class StableMap {
  using Map = UnorderedMap<K, V*, THash, TEqual, TAllocator>;

 public:
  StableMap() = default;
  ~StableMap() { clear(); }
  StableMap(const StableMap&) = delete;
  StableMap& operator=(const StableMap&) = delete;
  StableMap(StableMap&& o) noexcept : map_(base::move(o.map_)) {}
  StableMap& operator=(StableMap&& o) noexcept {
    if (this != &o) {
      clear();
      map_ = base::move(o.map_);
    }
    return *this;
  }

  // Insert-or-get: allocates a new V on first access.
  V& operator[](const K& key) {
    V** pp = map_.find(key);
    if (pp) return **pp;
    V* v = new V();
    map_[key] = v;
    return *v;
  }

  V* find(const K& key) {
    V** pp = map_.find(key);
    return pp ? *pp : nullptr;
  }
  const V* find(const K& key) const {
    const V* const* pp = map_.find(key);
    return pp ? *pp : nullptr;
  }

  bool contains(const K& key) const { return map_.contains(key); }

  bool erase(const K& key) {
    V** pp = map_.find(key);
    if (!pp) return false;
    delete *pp;
    map_.erase(key);
    return true;
  }

  void clear() {
    map_.ForEach([](const K&, V* v) { delete v; });
    map_.clear();
  }

  [[nodiscard]] mem_size size() const { return map_.size(); }
  [[nodiscard]] bool empty() const { return map_.empty(); }
  void reserve(mem_size n) { map_.reserve(n); }

  [[nodiscard]] mem_size remaining_capacity() const {
    return map_.remaining_capacity();
  }

  // --- Iterators (auto-dereference V* to V&) ------------------------------

  struct KeyValueRef {
    K& key;
    V& value;
  };
  struct ConstKeyValueRef {
    const K& key;
    const V& value;
  };

  class Iterator {
   public:
    using Inner = typename Map::Iterator;
    Iterator(Inner it) : it_(it) {}
    bool operator!=(const Iterator& o) const { return it_ != o.it_; }
    bool operator==(const Iterator& o) const { return it_ == o.it_; }
    Iterator& operator++() {
      ++it_;
      return *this;
    }
    KeyValueRef operator*() {
      auto kv = *it_;
      return {kv.key, *kv.value};
    }

   private:
    Inner it_;
  };

  class ConstIterator {
   public:
    using Inner = typename Map::ConstIterator;
    ConstIterator(Inner it) : it_(it) {}
    bool operator!=(const ConstIterator& o) const { return it_ != o.it_; }
    bool operator==(const ConstIterator& o) const { return it_ == o.it_; }
    ConstIterator& operator++() {
      ++it_;
      return *this;
    }
    ConstKeyValueRef operator*() const {
      auto kv = *it_;
      return {kv.key, *kv.value};
    }

   private:
    Inner it_;
  };

  Iterator begin() { return Iterator(map_.begin()); }
  Iterator end() { return Iterator(map_.end()); }
  ConstIterator begin() const { return ConstIterator(map_.begin()); }
  ConstIterator end() const { return ConstIterator(map_.end()); }

  // ForEach helper (matches UnorderedMap API)
  template <typename TFunc>
  void ForEach(TFunc&& func) {
    map_.ForEach([&func](const K& key, V* val) { func(key, *val); });
  }

  template <typename TFunc>
  void ForEach(TFunc&& func) const {
    map_.ForEach([&func](const K& key, V* val) { func(key, *val); });
  }

 private:
  Map map_;
};

}  // namespace base

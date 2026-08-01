// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::UnorderedSet - open-addressing hash set.
// Thin wrapper over UnorderedMap<K, bool> for set semantics.
#pragma once

#include <initializer_list>

#include <base/containers/unordered_map.h>

namespace base {

template <typename K, class THash = Hash<K>, class TEqual = Equal<K>,
          class TAllocator = DefaultAllocator>
class UnorderedSet {
 public:
  UnorderedSet() = default;

  // Brace initialization for static tables: {key, key, ...}.
  UnorderedSet(std::initializer_list<K> keys) {
    map_.reserve(keys.size());
    for (const K& key : keys) insert(key);
  }

  bool insert(const K& key) {
    auto result = map_.insert(key, true);
    return result.second;  // true if newly inserted
  }

  bool contains(const K& key) const { return map_.contains(key); }

  // 0 or 1, for callers written against the std::unordered_set spelling.
  [[nodiscard]] mem_size count(const K& key) const { return map_.contains(key) ? 1 : 0; }

  bool erase(const K& key) { return map_.erase(key); }

  void clear() { map_.clear(); }

  void reserve(mem_size count) { map_.reserve(count); }

  [[nodiscard]] mem_size size() const { return map_.size(); }
  [[nodiscard]] bool empty() const { return map_.empty(); }

  // Iterator that exposes only keys
  class Iterator {
   public:
    using MapIterator = typename UnorderedMap<K, bool, THash, TEqual, TAllocator>::Iterator;

    Iterator(MapIterator it) : it_(it) {}

    bool operator!=(const Iterator& other) const { return it_ != other.it_; }
    bool operator==(const Iterator& other) const { return it_ == other.it_; }

    Iterator& operator++() { ++it_; return *this; }

    const K& operator*() { return it_.key(); }

   private:
    MapIterator it_;
  };

  // Keys are immutable in a set, so const iteration walks the same Iterator
  // over the underlying map's const slots.
  class ConstIterator {
   public:
    using MapIterator = typename UnorderedMap<K, bool, THash, TEqual, TAllocator>::ConstIterator;

    ConstIterator(MapIterator it) : it_(it) {}

    bool operator!=(const ConstIterator& other) const { return it_ != other.it_; }
    bool operator==(const ConstIterator& other) const { return it_ == other.it_; }

    ConstIterator& operator++() {
      ++it_;
      return *this;
    }

    const K& operator*() const { return it_.key(); }

   private:
    MapIterator it_;
  };

  Iterator begin() { return Iterator(map_.begin()); }
  Iterator end() { return Iterator(map_.end()); }
  ConstIterator begin() const { return ConstIterator(map_.begin()); }
  ConstIterator end() const { return ConstIterator(map_.end()); }

  template <typename TFunc>
  void ForEach(TFunc&& func) const {
    map_.ForEach([&func](const K& key, const bool&) { func(key); });
  }

 private:
  UnorderedMap<K, bool, THash, TEqual, TAllocator> map_;
};

}  // namespace base

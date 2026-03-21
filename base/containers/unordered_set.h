// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::UnorderedSet - open-addressing hash set.
// Thin wrapper over UnorderedMap<K, bool> for set semantics.
#pragma once

#include <base/containers/unordered_map.h>

namespace base {

template <typename K, class THash = Hash<K>, class TEqual = Equal<K>,
          class TAllocator = DefaultAllocator>
class UnorderedSet {
 public:
  UnorderedSet() = default;

  bool insert(const K& key) {
    auto result = map_.insert(key, true);
    return result.b;  // true if newly inserted
  }

  bool contains(const K& key) const { return map_.contains(key); }

  bool erase(const K& key) { return map_.erase(key); }

  void clear() { map_.clear(); }

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

  Iterator begin() { return Iterator(map_.begin()); }
  Iterator end() { return Iterator(map_.end()); }

  template <typename TFunc>
  void ForEach(TFunc&& func) const {
    map_.ForEach([&func](const K& key, const bool&) { func(key); });
  }

 private:
  UnorderedMap<K, bool, THash, TEqual, TAllocator> map_;
};

}  // namespace base

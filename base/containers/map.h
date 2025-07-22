// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/containers/tree/red_black_tree_2.h>

namespace base {
template <typename Key, typename Value>
class Map {
 public:
  struct KeyValuePair {
    Key key;
    Value value;

    KeyValuePair(const Key& k, const Value& v) : key(k), value(v) {}

    explicit KeyValuePair(const Key& k) : key(k), value() {}
  };

  // The comparator must be able to compare a KeyValuePair with another.
  struct Comparator {
    static bool less_than(const KeyValuePair& lhs, const KeyValuePair& rhs) {
      return lhs.key < rhs.key;
    }
    static bool equals(const KeyValuePair& lhs, const KeyValuePair& rhs) {
      return lhs.key == rhs.key;
    }
  };

  Map() : size_(0) {}

  mem_size size() const { return size_; }

  bool Insert(const Key& key, const Value& value) {
    // tree.Insert returns true only if the element was not already there.
    if (tree.Insert(KeyValuePair(key, value))) {
      ++size_;
      return true;
    }
    return false;
  }
  bool Insert(const KeyValuePair& kvp) { return Insert(kvp.key, kvp.value); }

  bool Erase(const Key& key) {
    if (tree.Erase(KeyValuePair(key))) {
      --size_;
      return true;
    }
    return false;
  }

  bool Contains(const Key& key) const {
    return tree.Contains(KeyValuePair(key));
  }

  Value* Find(const Key& key) {
    auto* node = tree.Find(KeyValuePair(key));
    if (node != tree.nil()) {
      return &node->value.value;
    }
    return nullptr;
  }

  const Value* Find(const Key& key) const {
    auto* node = tree.Find(KeyValuePair(key));
    if (node != tree.nil()) {
      return &node->value.value;
    }
    return nullptr;
  }

 private:
  RedBlackTree2<KeyValuePair, Comparator> tree;
  mem_size size_;
};

}  // namespace base
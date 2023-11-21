// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/tree/red_black_tree_2.h>

namespace base {
template <typename Key, typename Value>
class Map {
 public:
  struct KeyValuePair {
    Key key;
    Value value;
    KeyValuePair(const Key& key, const Value& value) : key(key), value(value) {}
    bool operator<(const KeyValuePair& other) const { return key < other.key; }
  };

  void Insert(const Key& key, const Value& value) {
    tree.Insert(KeyValuePair(key, value));
  }
  void Insert(KeyValuePair& kvp) { tree.Insert(kvp); }

  void Erase(const Key& key) {
    // Implement logic to remove a key-value pair by key.
    // This will require a deletion method in RedBlackTree2.
  }

  bool Contains(const Key& key) const {
    // Implement logic to check if a key exists in the tree
    // This will require a search function modification in RedBlackTree2 to use
    // key comparison

    return tree.Search()
  }

  // Add other map-like functions such as erase, find, etc.

  // ...

 private:
  RedBlackTree2<KeyValuePair> tree;
};

}  // namespace base
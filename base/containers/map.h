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
    KeyValuePair(const Key& key, const Value& value) : key(key), value(value) {}
    bool operator<(const KeyValuePair& other) const { return key < other.key; }
  };

  struct Comparator {
    static bool less_than(const KeyValuePair& lhs, const KeyValuePair& rhs) {
      return lhs.key < rhs.key;
    }
    static bool equals(const KeyValuePair& lhs, const KeyValuePair& rhs) {
      return lhs.key == rhs.key;
    }
  };

  mem_size size() const { return size_; }

  bool Insert(const Key& key, const Value& value) {
    if (Contains(key)) {
      return false;
    }
    ++size_;
    tree.Insert(KeyValuePair(key, value));
  }
  void Insert(KeyValuePair& kvp) { Insert(kvp.key, kvp.value); }

  bool Erase(const Key& key) {
    if (!Contains(key))
      return false;

    auto* current = tree.root();
    while (current != nullptr) {
      if (key == current->value.key) {
        tree.Erase(current->value);
        return true;
      } else if (key < current->value.key) {
        current = current->left;
      } else {
        current = current->right;
      }
    }

    --size_;
    return true;
  }

  bool Contains(const Key& key) const {
    auto* current = tree.root();
    while (current != nullptr) {
      if (key == current->value.key) {
        return true;
      } else if (key < current->value.key) {
        current = current->left;
      } else {
        current = current->right;
      }
    }
    return false;
  }

 private:
  RedBlackTree2<KeyValuePair, Comparator> tree;
  mem_size size_;
};

}  // namespace base
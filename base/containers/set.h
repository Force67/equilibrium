// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/containers/tree/red_black_tree_2.h>

namespace base {

template <typename T>
class Set {
 public:
  Set() : size_(0) {}


  bool empty() const { return tree_.empty(); }
  mem_size size() const { return size_; }

  void Insert(const T& value) {
    // Avoid duplicates
    if (!tree_.Search(value)) {
      tree_.Insert(value);
    }
  }

  bool Remove(const T& value) {
    if (tree_.Remove(value)) {
      --size_;
      return true;
    }
    return false;
  }

  // Check if an element exists in the set
  bool Contains(const T& value) const { return tree_.Search(value); }

 private:
  base::RedBlackTree2<T> tree_;
  mem_size size_;
};

}  // namespace base

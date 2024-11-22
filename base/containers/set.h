// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <stack>
#include <base/arch.h>
#include <base/containers/tree/red_black_tree_2.h>

namespace base {

template <typename T>
class Set {
 public:
  Set() : size_(0) {}

  class Iterator {
   public:
    Iterator(typename RedBlackTree2<T>::Node* node) : current_(node) {
      while (current_ != nullptr && current_->left != nullptr) {
        path_.push(current_);
        current_ = current_->left;
      }
    }

    T& operator*() const { return current_->value; }

    Iterator& operator++() {
      if (current_->right != nullptr) {
        current_ = current_->right;
        while (current_->left != nullptr) {
          path_.push(current_);
          current_ = current_->left;
        }
      } else if (!path_.empty()) {
        current_ = path_.top();
        path_.pop();
      } else {
        current_ = nullptr;
      }
      return *this;
    }

    bool operator!=(const Iterator& other) const { return current_ != other.current_; }

   private:
    typename RedBlackTree2<T>::Node* current_;
    std::stack<typename RedBlackTree2<T>::Node*> path_;
  };

  Iterator begin() const { return Iterator(tree_.root()); }
  Iterator end() const { return Iterator(nullptr); }

  bool empty() const { return tree_.empty(); }
  mem_size size() const { return size_; }

  void Insert(const T& value) {
    // Avoid duplicates
    if (!tree_.Search(value)) {
      tree_.Insert(value);
    }
  }

  bool Remove(const T& value) {
    if (tree_.Erase(value)) {
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

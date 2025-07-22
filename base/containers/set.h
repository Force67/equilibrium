// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/containers/tree/red_black_tree_2.h>

namespace base {

template <typename T>
class Set {
 private:
  class IteratorImpl;

 public:
  using Iterator = IteratorImpl;

  Set() : size_(0) {}

  Iterator begin() const { return Iterator(tree_.root(), tree_.nil()); }
  Iterator end() const { return Iterator(tree_.nil(), tree_.nil()); }

  bool empty() const { return tree_.empty(); }
  mem_size size() const { return size_; }

  void Insert(const T& value) {
    if (tree_.Insert(value)) {
      ++size_;
    }
  }

  bool Remove(const T& value) {
    if (tree_.Erase(value)) {
      --size_;
      return true;
    }
    return false;
  }

  bool Contains(const T& value) const { return tree_.Contains(value); }

 private:
  base::RedBlackTree2<T> tree_;
  mem_size size_;

  class IteratorImpl {
   public:
    using Node = typename RedBlackTree2<T>::Node;

    IteratorImpl(Node* start_node, Node* nil_node) : nil_(nil_node) {
      // set up the first iterators state
      find_first(start_node);
    }

    T& operator*() const { return current_->value; }
    T* operator->() const { return current_->value; }

    IteratorImpl& operator++() {
      // If there is a right subtree, the successor is the smallest
      // element within that subtree.
      if (current_->right != nil_) {
        find_first(current_->right);
      }
      // Otherwise, the successor is the parent node from our stack.
      else if (path_top_ >= 0) {
        current_ = path_[path_top_--];  // Pop from stack
      }
      // If there's no right subtree and no parent, we've reached the end.
      else {
        current_ = nil_;
      }
      return *this;
    }

    bool operator!=(const IteratorImpl& other) const {
      return current_ != other.current_;
    }
    bool operator==(const IteratorImpl& other) const {
      return current_ == other.current_;
    }

   private:
    // Find the left-most (smallest) node in any given subtree
    void find_first(Node* node) {
      path_top_ = -1;  // Reset the stack for the new traversal
      current_ = node;

      // Traverse down the left spine, pushing every node onto the stack.
      while (current_ != nil_) {
        if (path_top_ < kMaxTreeDepth - 1) {
          path_[++path_top_] = current_;
        }
        current_ = current_->left;
      }

      // The stack now holds the full path. The top of the stack is the
      // left-most node, which is our first element.
      if (path_top_ >= 0) {
        current_ = path_[path_top_--];  // Pop to set current
      } else {
        // This case occurs if the initial node was nil (e.g., for end()).
        current_ = nil_;
      }
    }

    Node* current_;
    Node* nil_;

    static constexpr int kMaxTreeDepth = 128;
    Node* path_[kMaxTreeDepth];
    int path_top_;
  };
};

}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Set - ordered set over a red-black tree with a std::set-shaped API:
// in-order bidirectional iterators, insert/find/erase and lower/upper
// bounds. Node pointers are stable across mutation, so iterators stay valid
// for everything but the erased element.
#pragma once

#include <initializer_list>

#include <base/arch.h>
#include <base/containers/pair.h>
#include <base/containers/tree/red_black_tree_2.h>
#include <base/memory/move.h>

namespace base {

template <typename T>
class Set {
  using Tree = RedBlackTree2<T>;
  using Node = typename Tree::Node;

 public:
  using value_type = T;

  // Elements are keys: iteration never hands out a mutable reference.
  class Iterator {
   public:
    Iterator() : node_(nullptr), tree_(nullptr) {}
    Iterator(Node* node, const Tree* tree) : node_(node), tree_(tree) {}

    const T& operator*() const { return node_->value; }
    const T* operator->() const { return &node_->value; }

    Iterator& operator++() {
      node_ = tree_->Successor(node_);
      return *this;
    }
    Iterator operator++(int) {
      Iterator before = *this;
      node_ = tree_->Successor(node_);
      return before;
    }
    // From end() this steps to the last element.
    Iterator& operator--() {
      node_ = tree_->Predecessor(node_);
      return *this;
    }
    Iterator operator--(int) {
      Iterator before = *this;
      node_ = tree_->Predecessor(node_);
      return before;
    }

    bool operator==(const Iterator& other) const { return node_ == other.node_; }
    bool operator!=(const Iterator& other) const { return node_ != other.node_; }

   private:
    friend class Set;
    Node* node_;
    const Tree* tree_;
  };
  using iterator = Iterator;
  using const_iterator = Iterator;

  Set() : size_(0) {}

  Set(const Set& other) : tree_(other.tree_), size_(other.size_) {}
  Set& operator=(const Set& other) {
    if (this != &other) {
      tree_ = other.tree_;
      size_ = other.size_;
    }
    return *this;
  }

  Set(Set&& other) noexcept : tree_(base::move(other.tree_)), size_(other.size_) {
    other.size_ = 0;
  }
  Set& operator=(Set&& other) noexcept {
    if (this != &other) {
      tree_ = base::move(other.tree_);
      size_ = other.size_;
      other.size_ = 0;
    }
    return *this;
  }

  Set(std::initializer_list<T> values) : size_(0) {
    for (const T& value : values)
      insert(value);
  }

  Iterator begin() const {
    return Iterator(tree_.empty() ? tree_.nil() : tree_.Minimum(tree_.root()),
                    &tree_);
  }
  Iterator end() const { return Iterator(tree_.nil(), &tree_); }

  bool empty() const { return size_ == 0; }
  mem_size size() const { return size_; }

  Pair<Iterator, bool> insert(const T& value) {
    bool inserted = false;
    Node* node = tree_.FindOrInsert(value, &inserted);
    size_ += inserted;
    return {Iterator(node, &tree_), inserted};
  }
  Pair<Iterator, bool> insert(T&& value) {
    bool inserted = false;
    Node* node = tree_.FindOrInsert(base::move(value), &inserted);
    size_ += inserted;
    return {Iterator(node, &tree_), inserted};
  }

  Iterator find(const T& value) const {
    return Iterator(tree_.Find(value), &tree_);
  }
  bool contains(const T& value) const { return tree_.Contains(value); }
  [[nodiscard]] mem_size count(const T& value) const {
    return contains(value) ? 1 : 0;
  }

  // The first element not less than / greater than `value`.
  Iterator lower_bound(const T& value) const {
    return Iterator(tree_.LowerBound(value), &tree_);
  }
  Iterator upper_bound(const T& value) const {
    return Iterator(tree_.UpperBound(value), &tree_);
  }

  bool erase(const T& value) {
    if (!tree_.Erase(value))
      return false;
    --size_;
    return true;
  }
  // Returns the element after the erased one.
  Iterator erase(Iterator it) {
    Node* next = tree_.Successor(it.node_);
    if (tree_.Erase(*it))
      --size_;
    return Iterator(next, &tree_);
  }

  void clear() {
    tree_.Clear();
    size_ = 0;
  }

  // PascalCase spellings of the above.
  bool Insert(const T& value) { return insert(value).second; }
  bool Remove(const T& value) { return erase(value); }
  bool Contains(const T& value) const { return contains(value); }
  void Clear() { clear(); }

 private:
  Tree tree_;
  mem_size size_;
};

}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Map - ordered map over a red-black tree with a std::map-shaped
// API: in-order iterators yielding Pair<const Key, Value>, find/emplace/
// erase and operator[]. Node pointers are stable across mutation, so
// iterators stay valid for everything but the erased element.
//
// Lookups build a probe Pair, so Value must be default-constructible.
#pragma once

#include <base/arch.h>
#include <base/containers/pair.h>
#include <base/containers/tree/red_black_tree_2.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>

namespace base {

template <typename Key, typename Value>
class Map {
 public:
  using key_type = Key;
  using mapped_type = Value;
  using value_type = Pair<const Key, Value>;

  struct Comparator {
    static bool less_than(const value_type& lhs, const value_type& rhs) {
      return lhs.first < rhs.first;
    }
    static bool equals(const value_type& lhs, const value_type& rhs) {
      return lhs.first == rhs.first;
    }
  };

 private:
  using Tree = RedBlackTree2<value_type, Comparator>;
  using Node = typename Tree::Node;

 public:
  template <bool kConst>
  class IteratorBase {
   public:
    using reference = conditional_t<kConst, const value_type&, value_type&>;
    using pointer = conditional_t<kConst, const value_type*, value_type*>;

    IteratorBase() : node_(nullptr), tree_(nullptr) {}
    IteratorBase(Node* node, const Tree* tree) : node_(node), tree_(tree) {}

    // iterator converts to const_iterator.
    template <bool kOther>
      requires(kConst && !kOther)
    IteratorBase(const IteratorBase<kOther>& other)
        : node_(other.node_), tree_(other.tree_) {}

    reference operator*() const { return node_->value; }
    pointer operator->() const { return &node_->value; }

    IteratorBase& operator++() {
      node_ = tree_->Successor(node_);
      return *this;
    }
    IteratorBase operator++(int) {
      IteratorBase before = *this;
      node_ = tree_->Successor(node_);
      return before;
    }

    bool operator==(const IteratorBase& other) const {
      return node_ == other.node_;
    }
    bool operator!=(const IteratorBase& other) const {
      return node_ != other.node_;
    }

   private:
    friend class Map;
    template <bool>
    friend class IteratorBase;

    Node* node_;
    const Tree* tree_;
  };

  using iterator = IteratorBase<false>;
  using const_iterator = IteratorBase<true>;

  Map() : size_(0) {}

  Map(const Map& other) : tree_(other.tree_), size_(other.size_) {}

  Map& operator=(const Map& other) {
    if (this != &other) {
      tree_ = other.tree_;
      size_ = other.size_;
    }
    return *this;
  }

  // The moved-from map is left empty, not just stripped of its nodes.
  Map(Map&& other) noexcept : tree_(base::move(other.tree_)), size_(other.size_) {
    other.size_ = 0;
  }

  Map& operator=(Map&& other) noexcept {
    if (this != &other) {
      tree_ = base::move(other.tree_);
      size_ = other.size_;
      other.size_ = 0;
    }
    return *this;
  }

  mem_size size() const { return size_; }
  bool empty() const { return size_ == 0; }

  void clear() {
    tree_.Clear();
    size_ = 0;
  }

  iterator begin() { return iterator(FirstNode(), &tree_); }
  iterator end() { return iterator(tree_.nil(), &tree_); }
  const_iterator begin() const { return const_iterator(FirstNode(), &tree_); }
  const_iterator end() const { return const_iterator(tree_.nil(), &tree_); }

  iterator find(const Key& key) {
    return iterator(tree_.Find(Probe(key)), &tree_);
  }
  const_iterator find(const Key& key) const {
    return const_iterator(tree_.Find(Probe(key)), &tree_);
  }

  // 0 or 1, for callers written against the std::map spelling.
  [[nodiscard]] mem_size count(const Key& key) const { return contains(key) ? 1 : 0; }

  bool contains(const Key& key) const {
    return tree_.Contains(Probe(key));
  }

  Value& operator[](const Key& key) {
    return Emplace(key, Value()).first->second;
  }

  template <typename VV>
  Pair<iterator, bool> emplace(const Key& key, VV&& value) {
    return Emplace(key, base::forward<VV>(value));
  }

  Pair<iterator, bool> insert(const Key& key, const Value& value) {
    return Emplace(key, value);
  }

  bool erase(const Key& key) {
    if (tree_.Erase(Probe(key))) {
      --size_;
      return true;
    }
    return false;
  }

  // Erasing transplants surviving nodes instead of moving values, so the
  // successor computed up front stays valid.
  iterator erase(iterator it) {
    Node* next = tree_.Successor(it.node_);
    if (tree_.Erase(*it)) {
      --size_;
    }
    return iterator(next, &tree_);
  }

  // Pointer-returning lookups, kept for existing callers.
  bool Insert(const Key& key, const Value& value) {
    return Emplace(key, value).second;
  }
  bool Erase(const Key& key) { return erase(key); }
  bool Contains(const Key& key) const { return contains(key); }
  Value* Find(const Key& key) {
    Node* node = tree_.Find(Probe(key));
    return node != tree_.nil() ? &node->value.second : nullptr;
  }
  const Value* Find(const Key& key) const {
    return const_cast<Map*>(this)->Find(key);
  }

 private:
  static value_type Probe(const Key& key) { return value_type{key, Value()}; }

  Node* FirstNode() const {
    return tree_.empty() ? tree_.nil() : tree_.Minimum(tree_.root());
  }

  template <typename VV>
  Pair<iterator, bool> Emplace(const Key& key, VV&& value) {
    bool inserted = false;
    Node* node = tree_.FindOrInsert(value_type{key, base::forward<VV>(value)},
                                    &inserted);
    if (inserted) {
      ++size_;
    }
    return {iterator(node, &tree_), inserted};
  }

  Tree tree_;
  mem_size size_;
};

}  // namespace base

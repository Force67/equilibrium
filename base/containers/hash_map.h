// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::HashMap / base::HashSet - chained hash containers with the
// std::unordered_map / std::unordered_set contract: every element is a node
// of its own, so references and pointers to elements stay valid across
// inserts and rehashes, and iterators across everything but erasing the
// element they name. find/insert/emplace/erase(iterator) are std-shaped.
//
// Reach for base::UnorderedMap instead when element stability is not needed:
// open addressing keeps the elements inline and is faster to probe.
#pragma once

#include <initializer_list>

#include <base/arch.h>
#include <base/check.h>
#include <base/containers/container_traits.h>
#include <base/containers/pair.h>
#include <base/containers/unordered_map.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>

#include <new>

namespace base {

template <typename K,
          typename V,
          class THash = Hash<K>,
          class TEqual = Equal<K>,
          class TAllocator = DefaultAllocator>
class HashMap {
 public:
  using key_type = K;
  using mapped_type = V;
  using value_type = Pair<const K, V>;

 private:
  struct Node {
    // The pair is built in place, so a value that cannot move still fits.
    template <typename... TArgs>
    Node(mem_size h, const K& key, TArgs&&... args)
        : value{key, V(base::forward<TArgs>(args)...)}, hash(h) {}
    value_type value;
    Node* next = nullptr;
    mem_size hash;
  };

 public:
  template <bool kConst>
  class IteratorBase {
   public:
    using reference = conditional_t<kConst, const value_type&, value_type&>;
    using pointer = conditional_t<kConst, const value_type*, value_type*>;

    IteratorBase() = default;
    IteratorBase(Node* node, const HashMap* map) : node_(node), map_(map) {}
    template <bool kOther>
      requires(kConst && !kOther)
    IteratorBase(const IteratorBase<kOther>& other)
        : node_(other.node_), map_(other.map_) {}

    reference operator*() const { return node_->value; }
    pointer operator->() const { return &node_->value; }

    IteratorBase& operator++() {
      node_ = map_->NextNode(node_);
      return *this;
    }
    IteratorBase operator++(int) {
      IteratorBase before = *this;
      node_ = map_->NextNode(node_);
      return before;
    }

    bool operator==(const IteratorBase& other) const { return node_ == other.node_; }
    bool operator!=(const IteratorBase& other) const { return node_ != other.node_; }

   private:
    friend class HashMap;
    template <bool>
    friend class IteratorBase;
    Node* node_ = nullptr;
    const HashMap* map_ = nullptr;
  };
  using iterator = IteratorBase<false>;
  using const_iterator = IteratorBase<true>;

  HashMap() = default;
  HashMap(std::initializer_list<Pair<K, V>> entries) {
    reserve(entries.size());
    for (const auto& entry : entries)
      emplace(entry.first, entry.second);
  }
  HashMap(const HashMap& other) : hasher_(other.hasher_), equal_(other.equal_) {
    CopyFrom(other);
  }
  HashMap(HashMap&& other) noexcept { StealFrom(other); }
  HashMap& operator=(const HashMap& other) {
    if (this != &other) {
      Release();
      hasher_ = other.hasher_;
      equal_ = other.equal_;
      CopyFrom(other);
    }
    return *this;
  }
  HashMap& operator=(HashMap&& other) noexcept {
    if (this != &other) {
      Release();
      StealFrom(other);
    }
    return *this;
  }
  ~HashMap() { Release(); }

  iterator begin() { return iterator(FirstNode(), this); }
  iterator end() { return iterator(nullptr, this); }
  const_iterator begin() const { return const_iterator(FirstNode(), this); }
  const_iterator end() const { return const_iterator(nullptr, this); }

  [[nodiscard]] mem_size size() const { return size_; }
  [[nodiscard]] bool empty() const { return size_ == 0; }
  [[nodiscard]] mem_size bucket_count() const { return bucket_count_; }

  iterator find(const K& key) { return iterator(FindNode(key), this); }
  const_iterator find(const K& key) const {
    return const_iterator(FindNode(key), this);
  }
  [[nodiscard]] bool contains(const K& key) const { return FindNode(key) != nullptr; }
  [[nodiscard]] mem_size count(const K& key) const { return contains(key) ? 1 : 0; }

  // Constructs the value from |args| when |key| is absent; an existing entry
  // is left alone and the arguments are not consumed.
  template <typename... TArgs>
  Pair<iterator, bool> emplace(const K& key, TArgs&&... args) {
    const mem_size h = hasher_(key);
    if (Node* node = FindNode(key, h))
      return {iterator(node, this), false};
    return {iterator(Link(h, key, base::forward<TArgs>(args)...), this), true};
  }
  template <typename... TArgs>
  Pair<iterator, bool> try_emplace(const K& key, TArgs&&... args) {
    return emplace(key, base::forward<TArgs>(args)...);
  }

  template <typename A, typename B>
  Pair<iterator, bool> insert(const Pair<A, B>& entry) {
    return emplace(entry.first, entry.second);
  }
  template <typename A, typename B>
  Pair<iterator, bool> insert(Pair<A, B>&& entry) {
    return emplace(entry.first, base::move(entry.second));
  }

  // Replaces the value of an existing entry.
  template <typename VV>
  Pair<iterator, bool> insert_or_assign(const K& key, VV&& value) {
    if (Node* node = FindNode(key)) {
      node->value.second = base::forward<VV>(value);
      return {iterator(node, this), false};
    }
    return emplace(key, base::forward<VV>(value));
  }

  V& operator[](const K& key) { return emplace(key).first->second; }

  V& at(const K& key) {
    Node* node = FindNode(key);
    BASE_BUGCHECK(node != nullptr, "HashMap::at: key not present");
    return node->value.second;
  }
  const V& at(const K& key) const {
    return const_cast<HashMap*>(this)->at(key);
  }

  mem_size erase(const K& key) {
    if (!bucket_count_)
      return 0;
    const mem_size h = hasher_(key);
    Node** link = &buckets_[BucketOf(h)];
    for (Node* node = *link; node; link = &node->next, node = node->next) {
      if (node->hash == h && equal_(node->value.first, key)) {
        *link = node->next;
        Destroy(node);
        return 1;
      }
    }
    return 0;
  }

  // Returns the element after the erased one.
  iterator erase(const_iterator it) {
    Node* victim = it.node_;
    Node* next = NextNode(victim);
    Node** link = &buckets_[BucketOf(victim->hash)];
    while (*link != victim)
      link = &(*link)->next;
    *link = victim->next;
    Destroy(victim);
    return iterator(next, this);
  }
  iterator erase(iterator it) { return erase(const_iterator(it)); }

  void clear() {
    for (mem_size b = 0; b < bucket_count_; b++) {
      for (Node* node = buckets_[b]; node;) {
        Node* next = node->next;
        node->~Node();
        TAllocator::Free(node, sizeof(Node));
        node = next;
      }
      buckets_[b] = nullptr;
    }
    size_ = 0;
  }

  // Room for |count| elements without a rehash.
  void reserve(mem_size count) {
    if (count > bucket_count_)
      Rehash(count);
  }

 private:
  static mem_size RoundUpToPowerOfTwo(mem_size n) {
    mem_size p = 16;
    while (p < n)
      p *= 2;
    return p;
  }

  // Fibonacci hashing: the top bits of a multiplicative mix, so a hasher that
  // leaves structure in its low bits (an identity hash of aligned addresses)
  // still spreads over every bucket.
  mem_size BucketOf(mem_size h) const {
    return static_cast<mem_size>((static_cast<u64>(h) * 0x9E3779B97F4A7C15ull) >>
                                 (64 - bucket_shift_));
  }

  Node* FindNode(const K& key) const {
    return bucket_count_ ? FindNode(key, hasher_(key)) : nullptr;
  }
  Node* FindNode(const K& key, mem_size h) const {
    if (!bucket_count_)
      return nullptr;
    for (Node* node = buckets_[BucketOf(h)]; node; node = node->next)
      if (node->hash == h && equal_(node->value.first, key))
        return node;
    return nullptr;
  }

  Node* FirstNode() const {
    for (mem_size b = 0; b < bucket_count_; b++)
      if (buckets_[b])
        return buckets_[b];
    return nullptr;
  }

  Node* NextNode(Node* node) const {
    if (node->next)
      return node->next;
    for (mem_size b = BucketOf(node->hash) + 1; b < bucket_count_; b++)
      if (buckets_[b])
        return buckets_[b];
    return nullptr;
  }

  template <typename... TArgs>
  Node* Link(mem_size h, const K& key, TArgs&&... args) {
    if (size_ + 1 > bucket_count_)
      Rehash(size_ + 1);
    void* memory = TAllocator::Allocate(sizeof(Node));
    BASE_FATAL_CHECK(memory, "HashMap: node allocation failed");
    Node* node = ::new (memory) Node(h, key, base::forward<TArgs>(args)...);
    Node*& head = buckets_[BucketOf(h)];
    node->next = head;
    head = node;
    size_++;
    return node;
  }

  void Destroy(Node* node) {
    node->~Node();
    TAllocator::Free(node, sizeof(Node));
    size_--;
  }

  void Rehash(mem_size wanted) {
    const mem_size count = RoundUpToPowerOfTwo(wanted);
    u32 shift = 0;
    while ((mem_size(1) << shift) < count)
      shift++;
    auto* fresh = static_cast<Node**>(TAllocator::Allocate(count * sizeof(Node*)));
    BASE_FATAL_CHECK(fresh, "HashMap: bucket allocation failed");
    for (mem_size b = 0; b < count; b++)
      fresh[b] = nullptr;
    Node** old = buckets_;
    const mem_size old_count = bucket_count_;
    buckets_ = fresh;
    bucket_count_ = count;
    bucket_shift_ = shift;
    for (mem_size b = 0; b < old_count; b++) {
      for (Node* node = old[b]; node;) {
        Node* next = node->next;
        Node*& head = buckets_[BucketOf(node->hash)];
        node->next = head;
        head = node;
        node = next;
      }
    }
    if (old)
      TAllocator::Free(old, old_count * sizeof(Node*));
  }

  void CopyFrom(const HashMap& other) {
    reserve(other.size_);
    for (const auto& entry : other)
      emplace(entry.first, entry.second);
  }

  void StealFrom(HashMap& other) {
    buckets_ = other.buckets_;
    bucket_count_ = other.bucket_count_;
    bucket_shift_ = other.bucket_shift_;
    size_ = other.size_;
    hasher_ = other.hasher_;
    equal_ = other.equal_;
    other.buckets_ = nullptr;
    other.bucket_count_ = 0;
    other.bucket_shift_ = 0;
    other.size_ = 0;
  }

  void Release() {
    clear();
    if (buckets_)
      TAllocator::Free(buckets_, bucket_count_ * sizeof(Node*));
    buckets_ = nullptr;
    bucket_count_ = 0;
    bucket_shift_ = 0;
  }

  Node** buckets_ = nullptr;
  mem_size bucket_count_ = 0;
  u32 bucket_shift_ = 0;
  mem_size size_ = 0;
  THash hasher_{};
  TEqual equal_{};
};

template <typename K,
          class THash = Hash<K>,
          class TEqual = Equal<K>,
          class TAllocator = DefaultAllocator>
class HashSet {
  struct Empty {};
  using Map = HashMap<K, Empty, THash, TEqual, TAllocator>;

 public:
  using value_type = K;

  class Iterator {
   public:
    Iterator() = default;
    explicit Iterator(typename Map::const_iterator it) : it_(it) {}
    const K& operator*() const { return it_->first; }
    const K* operator->() const { return &it_->first; }
    Iterator& operator++() {
      ++it_;
      return *this;
    }
    Iterator operator++(int) {
      Iterator before = *this;
      ++it_;
      return before;
    }
    bool operator==(const Iterator& other) const { return it_ == other.it_; }
    bool operator!=(const Iterator& other) const { return it_ != other.it_; }

   private:
    friend class HashSet;
    typename Map::const_iterator it_;
  };
  using iterator = Iterator;
  using const_iterator = Iterator;

  HashSet() = default;
  HashSet(std::initializer_list<K> keys) {
    reserve(keys.size());
    for (const K& key : keys)
      insert(key);
  }

  Iterator begin() const { return Iterator(map_.begin()); }
  Iterator end() const { return Iterator(map_.end()); }

  [[nodiscard]] mem_size size() const { return map_.size(); }
  [[nodiscard]] bool empty() const { return map_.empty(); }

  Pair<Iterator, bool> insert(const K& key) {
    auto result = map_.emplace(key);
    return {Iterator(result.first), result.second};
  }
  Iterator find(const K& key) const { return Iterator(map_.find(key)); }
  [[nodiscard]] bool contains(const K& key) const { return map_.contains(key); }
  [[nodiscard]] mem_size count(const K& key) const { return map_.count(key); }

  mem_size erase(const K& key) { return map_.erase(key); }
  // Returns the element after the erased one.
  Iterator erase(Iterator it) { return Iterator(map_.erase(it.it_)); }

  void clear() { map_.clear(); }
  void reserve(mem_size count) { map_.reserve(count); }

 private:
  Map map_;
};

}  // namespace base

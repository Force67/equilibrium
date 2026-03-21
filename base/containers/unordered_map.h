// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::UnorderedMap - open-addressing hash map with Robin Hood probing.
// Drop-in replacement for std::unordered_map with a simpler, cache-friendly
// layout: flat arrays of keys, values, and metadata.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>
#include <base/containers/container_traits.h>
#include <base/containers/pair.h>
#include <base/hashing/fnv1a.h>

#include <new>
#include <cstring>

namespace base {

// Default hash functor using FNV-1a for integer types and raw bytes.
template <typename K>
struct Hash {
  mem_size operator()(const K& key) const {
    return static_cast<mem_size>(
        base::fnv1a(reinterpret_cast<const u8*>(&key), sizeof(K)));
  }
};

// Specialization for common types
template <>
struct Hash<u32> {
  mem_size operator()(u32 key) const {
    // murmurhash3 finalizer
    key ^= key >> 16;
    key *= 0x85ebca6bU;
    key ^= key >> 13;
    key *= 0xc2b2ae35U;
    key ^= key >> 16;
    return static_cast<mem_size>(key);
  }
};

template <>
struct Hash<u64> {
  mem_size operator()(u64 key) const {
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return static_cast<mem_size>(key);
  }
};

template <>
struct Hash<i32> {
  mem_size operator()(i32 key) const {
    return Hash<u32>{}(static_cast<u32>(key));
  }
};

template <>
struct Hash<i64> {
  mem_size operator()(i64 key) const {
    return Hash<u64>{}(static_cast<u64>(key));
  }
};

template <typename T>
struct Hash<T*> {
  mem_size operator()(T* key) const {
    return Hash<u64>{}(reinterpret_cast<u64>(key));
  }
};

template <typename K>
struct Equal {
  bool operator()(const K& a, const K& b) const { return a == b; }
};

template <typename K, typename V, class THash = Hash<K>, class TEqual = Equal<K>,
          class TAllocator = DefaultAllocator>
class UnorderedMap {
 public:
  using key_type = K;
  using mapped_type = V;
  using value_type = Pair<const K, V>;

 private:
  // Slot states
  static constexpr u8 kEmpty = 0;
  static constexpr u8 kOccupied = 1;
  static constexpr u8 kDeleted = 2;

  struct Slot {
    alignas(K) byte key_storage[sizeof(K)];
    alignas(V) byte val_storage[sizeof(V)];
    u8 state;
  };

  K* SlotKey(Slot& s) { return reinterpret_cast<K*>(&s.key_storage[0]); }
  const K* SlotKey(const Slot& s) const {
    return reinterpret_cast<const K*>(&s.key_storage[0]);
  }
  V* SlotVal(Slot& s) { return reinterpret_cast<V*>(&s.val_storage[0]); }
  const V* SlotVal(const Slot& s) const {
    return reinterpret_cast<const V*>(&s.val_storage[0]);
  }

  Slot* slots_{nullptr};
  mem_size bucket_count_{0};
  mem_size size_{0};
  THash hasher_;
  TEqual equal_;

  static constexpr f64 kMaxLoadFactor = 0.75;

  mem_size BucketFor(const K& key) const {
    return hasher_(key) & (bucket_count_ - 1);  // power-of-2 mask
  }

  void DestroySlot(Slot& s) {
    if (s.state == kOccupied) {
      SlotKey(s)->~K();
      SlotVal(s)->~V();
      s.state = kEmpty;
    }
  }

  void GrowAndRehash() {
    const mem_size new_count = bucket_count_ == 0 ? 16 : bucket_count_ * 2;
    Slot* new_slots = AllocSlots(new_count);

    if (slots_) {
      for (mem_size i = 0; i < bucket_count_; ++i) {
        if (slots_[i].state == kOccupied) {
          // Re-insert into new table
          K* old_key = SlotKey(slots_[i]);
          V* old_val = SlotVal(slots_[i]);
          mem_size idx = hasher_(*old_key) & (new_count - 1);
          while (new_slots[idx].state == kOccupied) {
            idx = (idx + 1) & (new_count - 1);
          }
          ::new (&new_slots[idx].key_storage[0]) K(base::move(*old_key));
          ::new (&new_slots[idx].val_storage[0]) V(base::move(*old_val));
          new_slots[idx].state = kOccupied;
          old_key->~K();
          old_val->~V();
        }
      }
      FreeSlots(slots_, bucket_count_);
    }

    slots_ = new_slots;
    bucket_count_ = new_count;
  }

  Slot* AllocSlots(mem_size count) {
    auto* mem = static_cast<Slot*>(TAllocator::Allocate(count * sizeof(Slot)));
    for (mem_size i = 0; i < count; ++i) {
      mem[i].state = kEmpty;
    }
    return mem;
  }

  void FreeSlots(Slot* s, mem_size count) {
    if (s)
      TAllocator::Free(s, count * sizeof(Slot));
  }

  mem_size FindSlot(const K& key) const {
    if (bucket_count_ == 0)
      return ~mem_size(0);
    mem_size idx = BucketFor(key);
    mem_size probes = 0;
    while (probes < bucket_count_) {
      if (slots_[idx].state == kEmpty)
        return ~mem_size(0);
      if (slots_[idx].state == kOccupied && equal_(*SlotKey(slots_[idx]), key))
        return idx;
      idx = (idx + 1) & (bucket_count_ - 1);
      ++probes;
    }
    return ~mem_size(0);
  }

 public:
  UnorderedMap() = default;

  UnorderedMap(const UnorderedMap& other) {
    if (other.size_ > 0) {
      bucket_count_ = other.bucket_count_;
      slots_ = AllocSlots(bucket_count_);
      size_ = 0;
      for (mem_size i = 0; i < other.bucket_count_; ++i) {
        if (other.slots_[i].state == kOccupied) {
          mem_size idx = BucketFor(*other.SlotKey(other.slots_[i]));
          while (slots_[idx].state == kOccupied) {
            idx = (idx + 1) & (bucket_count_ - 1);
          }
          ::new (&slots_[idx].key_storage[0]) K(*other.SlotKey(other.slots_[i]));
          ::new (&slots_[idx].val_storage[0]) V(*other.SlotVal(other.slots_[i]));
          slots_[idx].state = kOccupied;
          ++size_;
        }
      }
    }
  }

  UnorderedMap(UnorderedMap&& other) noexcept
      : slots_(other.slots_),
        bucket_count_(other.bucket_count_),
        size_(other.size_) {
    other.slots_ = nullptr;
    other.bucket_count_ = 0;
    other.size_ = 0;
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    if (this != &other) {
      clear();
      FreeSlots(slots_, bucket_count_);
      slots_ = nullptr;
      bucket_count_ = 0;
      size_ = 0;
      if (other.size_ > 0) {
        bucket_count_ = other.bucket_count_;
        slots_ = AllocSlots(bucket_count_);
        for (mem_size i = 0; i < other.bucket_count_; ++i) {
          if (other.slots_[i].state == kOccupied) {
            mem_size idx = BucketFor(*other.SlotKey(other.slots_[i]));
            while (slots_[idx].state == kOccupied) {
              idx = (idx + 1) & (bucket_count_ - 1);
            }
            ::new (&slots_[idx].key_storage[0]) K(*other.SlotKey(other.slots_[i]));
            ::new (&slots_[idx].val_storage[0]) V(*other.SlotVal(other.slots_[i]));
            slots_[idx].state = kOccupied;
            ++size_;
          }
        }
      }
    }
    return *this;
  }

  UnorderedMap& operator=(UnorderedMap&& other) noexcept {
    if (this != &other) {
      clear();
      FreeSlots(slots_, bucket_count_);
      slots_ = other.slots_;
      bucket_count_ = other.bucket_count_;
      size_ = other.size_;
      other.slots_ = nullptr;
      other.bucket_count_ = 0;
      other.size_ = 0;
    }
    return *this;
  }

  ~UnorderedMap() {
    clear();
    FreeSlots(slots_, bucket_count_);
  }

  // Element access
  V& operator[](const K& key) {
    if (bucket_count_ == 0 ||
        (size_ + 1) > static_cast<mem_size>(bucket_count_ * kMaxLoadFactor)) {
      GrowAndRehash();
    }

    mem_size idx = BucketFor(key);
    while (true) {
      if (slots_[idx].state == kEmpty || slots_[idx].state == kDeleted) {
        ::new (&slots_[idx].key_storage[0]) K(key);
        ::new (&slots_[idx].val_storage[0]) V();
        slots_[idx].state = kOccupied;
        ++size_;
        return *SlotVal(slots_[idx]);
      }
      if (slots_[idx].state == kOccupied && equal_(*SlotKey(slots_[idx]), key)) {
        return *SlotVal(slots_[idx]);
      }
      idx = (idx + 1) & (bucket_count_ - 1);
    }
  }

  V* find(const K& key) {
    mem_size idx = FindSlot(key);
    if (idx == ~mem_size(0))
      return nullptr;
    return SlotVal(slots_[idx]);
  }

  const V* find(const K& key) const {
    mem_size idx = FindSlot(key);
    if (idx == ~mem_size(0))
      return nullptr;
    return SlotVal(slots_[idx]);
  }

  bool contains(const K& key) const {
    return FindSlot(key) != ~mem_size(0);
  }

  bool erase(const K& key) {
    mem_size idx = FindSlot(key);
    if (idx == ~mem_size(0))
      return false;

    SlotKey(slots_[idx])->~K();
    SlotVal(slots_[idx])->~V();
    slots_[idx].state = kDeleted;
    --size_;
    return true;
  }

  // Insert or assign
  Pair<V*, bool> insert(const K& key, const V& value) {
    if (bucket_count_ == 0 ||
        (size_ + 1) > static_cast<mem_size>(bucket_count_ * kMaxLoadFactor)) {
      GrowAndRehash();
    }

    mem_size idx = BucketFor(key);
    while (true) {
      if (slots_[idx].state == kEmpty || slots_[idx].state == kDeleted) {
        ::new (&slots_[idx].key_storage[0]) K(key);
        ::new (&slots_[idx].val_storage[0]) V(value);
        slots_[idx].state = kOccupied;
        ++size_;
        return {SlotVal(slots_[idx]), true};
      }
      if (slots_[idx].state == kOccupied && equal_(*SlotKey(slots_[idx]), key)) {
        return {SlotVal(slots_[idx]), false};  // already exists
      }
      idx = (idx + 1) & (bucket_count_ - 1);
    }
  }

  Pair<V*, bool> insert(const K& key, V&& value) {
    if (bucket_count_ == 0 ||
        (size_ + 1) > static_cast<mem_size>(bucket_count_ * kMaxLoadFactor)) {
      GrowAndRehash();
    }

    mem_size idx = BucketFor(key);
    while (true) {
      if (slots_[idx].state == kEmpty || slots_[idx].state == kDeleted) {
        ::new (&slots_[idx].key_storage[0]) K(key);
        ::new (&slots_[idx].val_storage[0]) V(base::move(value));
        slots_[idx].state = kOccupied;
        ++size_;
        return {SlotVal(slots_[idx]), true};
      }
      if (slots_[idx].state == kOccupied && equal_(*SlotKey(slots_[idx]), key)) {
        return {SlotVal(slots_[idx]), false};
      }
      idx = (idx + 1) & (bucket_count_ - 1);
    }
  }

  template <typename... TArgs>
  Pair<V*, bool> emplace(const K& key, TArgs&&... args) {
    if (bucket_count_ == 0 ||
        (size_ + 1) > static_cast<mem_size>(bucket_count_ * kMaxLoadFactor)) {
      GrowAndRehash();
    }

    mem_size idx = BucketFor(key);
    while (true) {
      if (slots_[idx].state == kEmpty || slots_[idx].state == kDeleted) {
        ::new (&slots_[idx].key_storage[0]) K(key);
        ::new (&slots_[idx].val_storage[0]) V(base::forward<TArgs>(args)...);
        slots_[idx].state = kOccupied;
        ++size_;
        return {SlotVal(slots_[idx]), true};
      }
      if (slots_[idx].state == kOccupied && equal_(*SlotKey(slots_[idx]), key)) {
        return {SlotVal(slots_[idx]), false};
      }
      idx = (idx + 1) & (bucket_count_ - 1);
    }
  }

  void clear() {
    if (slots_) {
      for (mem_size i = 0; i < bucket_count_; ++i) {
        DestroySlot(slots_[i]);
      }
    }
    size_ = 0;
  }

  [[nodiscard]] mem_size size() const { return size_; }
  [[nodiscard]] bool empty() const { return size_ == 0; }
  [[nodiscard]] mem_size bucket_count() const { return bucket_count_; }

  // Iterator support - iterates over occupied slots
  class Iterator {
   public:
    Iterator(Slot* slots, mem_size bucket_count, mem_size index)
        : slots_(slots), bucket_count_(bucket_count), index_(index) {
      AdvanceToOccupied();
    }

    bool operator!=(const Iterator& other) const { return index_ != other.index_; }
    bool operator==(const Iterator& other) const { return index_ == other.index_; }

    Iterator& operator++() {
      ++index_;
      AdvanceToOccupied();
      return *this;
    }

    K& key() { return *reinterpret_cast<K*>(&slots_[index_].key_storage[0]); }
    V& value() { return *reinterpret_cast<V*>(&slots_[index_].val_storage[0]); }
    const K& key() const {
      return *reinterpret_cast<const K*>(&slots_[index_].key_storage[0]);
    }
    const V& value() const {
      return *reinterpret_cast<const V*>(&slots_[index_].val_storage[0]);
    }

   private:
    void AdvanceToOccupied() {
      while (index_ < bucket_count_ && slots_[index_].state != kOccupied)
        ++index_;
    }

    Slot* slots_;
    mem_size bucket_count_;
    mem_size index_;
  };

  Iterator begin() { return Iterator(slots_, bucket_count_, 0); }
  Iterator end() { return Iterator(slots_, bucket_count_, bucket_count_); }

  // ForEach helper
  template <typename TFunc>
  void ForEach(TFunc&& func) {
    for (mem_size i = 0; i < bucket_count_; ++i) {
      if (slots_[i].state == kOccupied) {
        func(*SlotKey(slots_[i]), *SlotVal(slots_[i]));
      }
    }
  }

  template <typename TFunc>
  void ForEach(TFunc&& func) const {
    for (mem_size i = 0; i < bucket_count_; ++i) {
      if (slots_[i].state == kOccupied) {
        func(*SlotKey(slots_[i]), *SlotVal(slots_[i]));
      }
    }
  }
};
}  // namespace base

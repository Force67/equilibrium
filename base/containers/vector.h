// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// std::vector replacement.
#pragma once

#include <memory>
#include <type_traits>

#include <base/arch.h>
#include <base/check.h>

namespace base {

template <typename T>
inline void destruct_range(T first, T last) {
  if constexpr (!std::is_trivial<T>()) {
    for (; first != last; ++first)
      (*first).~T();
  }
}

// TODO: shrink_to_fit, resize

template <typename T, class TAllocator>
class Vector {
 public:
  // indicates how much to overallocate
  constexpr static mem_size kDefaultMult = 2;

  Vector() : data_(nullptr), end_(nullptr), capacity_(nullptr) {}

  Vector(mem_size reserve_count) {
    data_ = end_ = Vector::Allocate(reserve_count);
    capacity_ = &data_[reserve_count];
  }

  ~Vector() {
    // clear all without resetting pointers
    destruct_range(data_, end_);
    Vector::Free(data_, capacity());
  }

  // TBD
  void resize(mem_size new_capacity, const T& value) {
    IMPOSSIBLE;
#if 0
    if (new_capacity > size()) [[likely]]
      IMPOSSIBLE;
      //InsertAtEnd(new_capacity - size(), value);
    else
      IMPOSSIBLE;
#endif
  }

  // increases internal capacity
  void reserve(mem_size new_reserved_capacity) {
    if (new_reserved_capacity > capacity()) [[likely]]
      GrowCapacity(new_reserved_capacity);
  }

  // reduces .capacity to .size
  mem_size shrink_to_fit() {
    if (end_ != capacity_) [[likely]] {
      if (data_ == end_) {
        // nothing to do, just clear everything out.
        ReleaseAll();
      } else {
        // TODO: re allocate in place...
        IMPOSSIBLE;
      }
    }
  }

  void push_back(const T& value) {
    // cram it into pre over reserved space
    if (end_ < capacity_) [[likely]]
      ::new (static_cast<void*>(end_++)) T(value);
    else
      InsertAtEnd(value);
  }

  template <typename... TArgs>
  T& emplace_back(TArgs&&... args) {
    if (end_ < capacity_) [[likely]]
      ::new (static_cast<void*>(end_++)) T(args...);
    else
      InsertAtEnd(args...);
    return back();
  }

  T* at(mem_size pos) {
    T* dest = &data_[pos];
    return dest > end_ ? nullptr : dest;
  }

  bool erase(mem_size pos) {
    T* dest = &data_[pos];
    if (dest == (end_ - 1)) {
      --end_;
      end_->~T();
      return true;
    }

    const T* source = &data_[pos + 1];
    if (dest > end_ || source > end_)
      return false;

    // if we remove in the middle, we memmove the upper objects down by one place.
    std::memmove(dest, source, end_ - source);
    --end_;
    end_->~T();
    return true;
  }

  const T& back() const {
    DCHECK(empty());
    return *(end_ - 1);
  }

  T& back() {
    DCHECK(empty());
    return *(end_ - 1);
  }

  T* begin() const { return data_; }
  T* end() const { return end_; }

  bool empty() const { return data_ == nullptr || end_ <= data_; }
  mem_size size() const { return end_ - data_; }
  mem_size capacity() const { return capacity_ - data_; }

 private:
  mem_size CalculateNewCapacity(mem_size cap) {
    return cap > 0 ? cap * /*capacity_mult_*/ kDefaultMult : 1;
  }

  template <typename... TArgs>
  void InsertAtEnd(TArgs&&... args) {
    mem_size current_cap = size();
    mem_size new_cap = CalculateNewCapacity(current_cap);

    GrowCapacity(current_cap, new_cap);
    // insert at end
    ::new (static_cast<void*>(end_++)) T(args...);
  }

  void GrowCapacity(mem_size current_cap, mem_size new_cap) {
    // remember: param is cap not size.
    T* new_block = Vector::Allocate(new_cap);

    std::memcpy(new_block, data_, current_cap * sizeof(T));
    destruct_range(data_, end_);
    Vector::Free(data_, current_cap);

    data_ = new_block;
    end_ = &new_block[current_cap];
    capacity_ = &new_block[new_cap];
  }

  void ReleaseAll() {
    if (data_ && end_ && capacity_) {
      destruct_range(data_, end_);
      Vector::Free(data_, capacity());
      data_ = end_ = capacity_ = nullptr;
    }
  }

  // memory primitives for cap sizes
  T* Allocate(mem_size capacity) {
    return static_cast<T*>(TAllocator::Allocate(capacity * sizeof(T)));
  }
  void Free(T* block, mem_size n) {
    TAllocator::Free(static_cast<void*>(block), n * sizeof(T));
  }

 private:
  T* data_;
  T* end_;
  T* capacity_;
  // mem_size capacity_mult_;
};
}  // namespace base

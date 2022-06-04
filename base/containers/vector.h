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

template <typename T, class TAllocatorTraits>
class Vector {
 public:
  // use capacity_multiplicator to indicate how much to overallocate
  Vector(mem_size reserve_count, mem_size capacity_multiplicator = 2)
      : capacity_mult_(capacity_multiplicator) {
    data_ = end_ = Vector::Allocate(reserve_count);
    capacity_ = &data_[reserve_count];
  }

  ~Vector() {
    destruct_range(data_, end_);
    Vector::Free(data_, capacity());
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
    if (dest > end_)
      return nullptr;
    return dest;
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
    return cap > 0 ? cap * capacity_mult_ : 1;
  }

  template <typename... TArgs>
  void InsertAtEnd(TArgs&&... args) {
    mem_size current_cap = size();
    mem_size new_cap = CalculateNewCapacity(current_cap);
    // remember: param is cap not size.
    T* new_block = Vector::Allocate(new_cap);

    // move existing data
    std::memcpy(new_block, data_, current_cap * sizeof(T));
    destruct_range(data_, end_);
    Vector::Free(data_, current_cap);

    data_ = new_block;
    end_ = &new_block[current_cap];
    capacity_ = &new_block[new_cap];

    ::new (static_cast<void*>(end_++)) T(args...);
  }

  // memory primitives for cap sizes
  T* Allocate(mem_size capacity) {
    return static_cast<T*>(allocator_.Allocate(capacity * sizeof(T)));
  }
  void Free(T* block, mem_size n) {
    allocator_.Free(static_cast<void*>(block), n * sizeof(T));
  }

 private:
  T* data_{nullptr};
  T* end_{nullptr};
  T* capacity_{nullptr};
  mem_size capacity_mult_{2};
  TAllocatorTraits allocator_;
};
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Vector is a sequence container that encapsulates dynamic size arrays
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>
#include <base/containers/container_traits.h>

#include <new>      // < for placement new
#include <cstring>  // < linux memory stuff
#include <initializer_list>

namespace base {

enum class VectorReservePolicy {
  kForPushback,  // < This optimization allows you to utilize push_back without
                 // immediately increasing the capacity, reserving additional space
                 // only when necessary.
  kForData,      // < This reserve operation functions similarly to what you're
             // accustomed to with std::vector. It preallocates capacity, effectively
             // simulating the insertion of a number of "empty" elements. If you
             // intend to copy data, especially using .data(), opt for this approach.
};

template <typename T, class TAllocator = base::DefaultAllocator>
class Vector {
 public:
  // indicates how much to overallocate
  constexpr static mem_size kDefaultMult = 2;

  Vector() : data_(nullptr), end_(nullptr), capacity_(nullptr) {}

  Vector(mem_size reserve_count,
         // we default to nothing for the policy since we want the user to think
         // about what they are doing.
         const VectorReservePolicy policy) {
    data_ = Vector::Allocate(reserve_count);
    capacity_ = &data_[reserve_count];
    end_ = policy == VectorReservePolicy::kForPushback ? data_ : capacity_;
  }

  // from braces {}
  Vector(std::initializer_list<T> list) {
    const auto size = list.size();
    data_ = Vector::Allocate(size);
    capacity_ = &data_[size];
    end_ = capacity_;
    for (auto&& item : list) {
      ::new (static_cast<void*>(end_++)) T(item);
    }
  }

  // move constructor
  Vector(Vector&& other) {
    data_ = other.data_;
    end_ = other.end_;
    capacity_ = other.capacity_;
    other.data_ = nullptr;
    other.end_ = nullptr;
    other.capacity_ = nullptr;
  }

  ~Vector() {
    // clear all without resetting pointers
    base::DestructRange(data_, end_);
    Vector::Free(data_, capacity());
  }

  Vector& operator=(Vector&& other) noexcept {
    if (this != &other) {
      base::DestructRange(data_, end_);
      Free(data_, capacity());

      data_ = other.data_;
      end_ = other.end_;
      capacity_ = other.capacity_;

      other.data_ = nullptr;
      other.end_ = nullptr;
      other.capacity_ = nullptr;
    }
    return *this;
  }

  void resize(mem_size new_capacity, const T& value) {
    if (new_capacity > size()) [[likely]]
      InsertValueAtEnd(new_capacity - size(), value);
    else {
      base::DestructRange(data_ + new_capacity, end_);
      end_ = data_ + new_capacity;
    }
  }

  void resize(mem_size new_capacity) {
    if (new_capacity > size()) [[likely]]
      InsertNValuesAtEnd(new_capacity - size());
    else {
      base::DestructRange(data_ + new_capacity, end_);
      end_ = data_ + new_capacity;
    }
  }

  // increase internal capacity
  void reserve(mem_size new_reserved_capacity) {
    DCHECK(new_reserved_capacity != 0 && capacity() != 0,
           "Vector::reserve: Use resize instead of resize for populating an empty "
           "Vector");

    if (new_reserved_capacity > capacity()) [[likely]]
      GrowCapacity(capacity(), new_reserved_capacity);
  }

  // reduces .capacity to .size
  mem_size shrink_to_fit() {
    if (end_ != capacity_) [[likely]] {
      if (data_ == end_) {
        // nothing to do, just clear everything out.
        ReleaseAll();
      } else {
        const auto current_size = size();
        T* new_block = Allocate(current_size);

        auto* src = data_;
        auto* dest = new_block;

        for (; src != end_; ++src, ++dest) {
          ::new (reinterpret_cast<void*>(dest)) T(base::move(*src));
        }

        base::DestructRange(data_, end_);
        Free(data_, capacity());

        data_ = new_block;
        end_ = &new_block[current_size];
        capacity_ = &new_block[current_size];
      }
    }
    return size();
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
      ::new (static_cast<void*>(end_++)) T(base::forward<TArgs>(args)...);
    else
      InsertAtEnd(base::forward<TArgs>(args)...);
    return back();
  }

  [[nodiscard]] T* at(mem_size pos) const {
    T* dest = &data_[pos];
    return dest > end_ ? nullptr : dest;
  }

  [[nodiscard]] T* find(const T& element_match) const {
	mem_size left = 0;
	mem_size right = size() - 1;

	while (left <= right) {
	  mem_size middle = left + (right - left) / 2;
	  T& middle_element = *(begin() + middle);

	  if (middle_element == element_match)
		return &middle_element;
	  else if (middle_element < element_match)
		left = middle + 1;
	  else
		right = middle - 1;
	}

	return nullptr;
  }

  // single element at a specified position.
  T* insert(T* pos, const T& value) {
    auto index = pos - begin();
    if (end_ == capacity_) {  // Need to grow the vector
      size_t newCapacity = size() == 0 ? 1 : size() * kDefaultMult;
      reserve(newCapacity);
    }
    if (pos != end_) {
      // Shift elements to the right
      for (auto it = end_; it != pos; --it) {
        *it = base::move(*(it - 1));
      }
    }
    // Construct the new element
    ::new (static_cast<void*>(&*pos)) T(value);
    ++end_;
    return begin() + index;
  }

  // multiple elements of the same value at a specified position.
  void insert(T* pos, size_t count, const T& value) {
    if (count == 0)
      return;
    auto index = pos - begin();
    while (size() + count > capacity()) {  // Ensure capacity
      reserve(size() == 0 ? count : size() * kDefaultMult);
    }
    // Move existing elements to make space
    for (auto it = end_ + count - 1; it >= pos + count; --it) {
      *it = base::move(*(it - count));
    }
    // Insert new elements
    for (auto it = pos; it != pos + count; ++it) {
	  ::new (static_cast<void*>(&*it)) T(value);
	}
    end_ += count;
  }

  // range of elements at a specified position.
  template <class InputIt>
  void insert(T* pos, InputIt first, InputIt last) {
    auto distance = first - last;
    if (distance <= 0)
      return;
    auto index = pos - begin();
    while (size() + distance > capacity()) {  // Ensure capacity
      reserve(size() == 0 ? distance : size() * kDefaultMult);
    }
    // Move existing elements to make space
    for (auto it = end_ + distance - 1; it >= pos + distance; --it) {
      *it = base::move(*(it - distance));
    }
    
    // Copy new elements
    memcpy(pos, first, distance * sizeof(T));
    //std::copy(first, last, pos);
    end_ += distance;
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
    memmove(dest, source, end_ - source);
    --end_;
    end_->~T();
    return true;
  }

   bool erase(T* element_ptr) {
    if (element_ptr < data_ || element_ptr >= end_) {
      return false;  // Pointer is out of bounds
    }

    // Move the elements after the erased element
    T* next = element_ptr + 1;
    if (next != end_) {
      base::move(next, end_, element_ptr);
    }

    // Destroy the last element since it is now a duplicate
    --end_;
    end_->~T();
    return true;
  }

  void clear() noexcept {
    base::DestructRange(data_, end_);
    end_ = data_;
  }

  void reset() {
    // clear all without resetting pointers
    base::DestructRange(data_, end_);
    Vector::Free(data_, capacity());

    data_ = nullptr;
    end_ = nullptr;
    capacity_ = nullptr;
  }

  T& front() {
    DCHECK(!empty(), "Vector is empty.");
    return data_[0];
  }

  // Access first element (const)
  const T& front() const {
    DCHECK(!empty(), "Vector is empty.");
    return data_[0];
  }

  [[nodiscard]] const T& back() const {
    DCHECK(!empty());
    return *(end_ - 1);
  }

  [[nodiscard]] T& back() {
    DCHECK(!empty());
    return *(end_ - 1);
  }

  [[nodiscard]] T* begin() const { return data_; }
  [[nodiscard]] T* end() const { return end_; }
  [[nodiscard]] T* data() const { return data_; }

  [[nodiscard]] bool empty() const { return data_ == nullptr || end_ == data_; }
  [[nodiscard]] mem_size size() const { return end_ - data_; }
  [[nodiscard]] mem_size capacity() const { return capacity_ - data_; }

  [[nodiscard]] CONSTEXPR_ND T& operator[](mem_size pos) const {
    DCHECK(pos <= size(), "Vector::[]: Access out of bounds");
    return *(Vector::at(pos));
  }

  bool Contains(const T& element_match) const {
    mem_size left = 0;
    mem_size right = size() - 1;

    while (left <= right) {
      mem_size middle = left + (right - left) / 2;
      const T& middle_element = *(begin() + middle);

      if (middle_element == element_match)
        return true;
      else if (middle_element < element_match)
        left = middle + 1;
      else
        right = middle - 1;
    }

    return false;
  }

 private:
  mem_size CalculateNewCapacity(mem_size cap) {
    return cap > 0 ? cap * /*capacity_mult_*/ kDefaultMult : 1;
  }

  template <typename... TArgs>
  void InsertAtEnd(TArgs&&... args) {
    const auto current_cap = size();
    const auto new_cap = CalculateNewCapacity(current_cap);

    GrowCapacity(current_cap, new_cap);
    // insert at end
    ::new (static_cast<void*>(end_++)) T(base::forward<TArgs>(args)...);
  }

  void InsertValueAtEnd(mem_size n, const T& value) {
    if (n > mem_size(capacity_ - end_)) {
      const auto current_cap = size();
      const auto grow_size = CalculateNewCapacity(current_cap);
      const auto new_size =
          grow_size < current_cap + n ? current_cap + n : grow_size;

      GrowCapacity(current_cap, new_size);

      T* formal_ptr = &data_[current_cap];

      for (auto i = current_cap; i < new_size; i++) {
        ::new (static_cast<void*>(formal_ptr++)) T(value);
      }

      // TODO: this should be done nicer.
      end_ = &data_[new_size];
    }
  }

  void InsertNValuesAtEnd(mem_size n) {
    if (n > mem_size(capacity_ - end_)) {
      const auto current_cap = size();
      const auto grow_size = CalculateNewCapacity(current_cap);
      const auto new_size =
          grow_size < current_cap + n ? current_cap + n : grow_size;

      GrowCapacity(current_cap, new_size);

      T* formal_ptr = &data_[current_cap];
      for (auto i = current_cap; i < new_size; i++) {
        ::new (static_cast<void*>(formal_ptr++)) T();
      }

      // TODO: this should be done nicer.
      end_ = &data_[new_size];
    }
  }

  void GrowCapacity(mem_size current_cap, mem_size new_cap) {
    // remember: param is cap not size.
    T* new_block = Vector::Allocate(new_cap);

    if (data_) {
      // manually move construct at new place!
      // https://github.com/electronicarts/EASTL/blob/db160651d4f980c04d260cece06edee00c10bb33/include/EASTL/memory.h#L702
      // basically this, it could be moved to its own sub later.
      {
        auto* first = data_;
        auto* last = end_;

        auto* new_spot = new_block;
        for (; first != last; ++first, ++new_spot) {
          ::new (reinterpret_cast<void*>(new_spot)) T(base::move(*first));
        }
      }
      base::DestructRange(data_, end_);
      Vector::Free(data_, current_cap);
    }

    data_ = new_block;
    end_ = &new_block[current_cap];
    capacity_ = &new_block[new_cap];
  }

  void ReleaseAll() {
    if (data_ && end_ && capacity_) {
      base::DestructRange(data_, end_);
      Vector::Free(data_, capacity());
      data_ = end_ = capacity_ = nullptr;
    }
  }

  // memory primitives for cap sizes
  T* Allocate(mem_size capacity) {
    return static_cast<T*>(TAllocator::Allocate(capacity * sizeof(T)));
  }
  void Free(T* block, mem_size n) {
    TAllocator::Free(reinterpret_cast<void*>(block), n * sizeof(T));
  }

 private:
  T* data_;
  T* end_;
  T* capacity_;
  // mem_size capacity_mult_;
};
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Vector is a sequence container that encapsulates dynamic size arrays
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>
#include <base/math/value_bounds.h>
#include <base/containers/container_traits.h>

#include <new>      // < for placement new
#include <cstring>  // < linux memory stuff
#include <initializer_list>

namespace base {

enum class VectorReservePolicy {
  kForPushback,  // < This optimization allows you to utilize push_back without
                 // immediately increasing the capacity, reserving additional
                 // space only when necessary.
  kForData,      // < This reserve operation functions similarly to what you're
                 // accustomed to with std::vector. It preallocates capacity,
                 // effectively simulating the insertion of a number of "empty"
  // elements. If you intend to copy data, especially using .data(),
  // opt for this approach.
};

template <typename T, class TAllocator = base::DefaultAllocator>
class Vector {
 public:
  using value_type = T;
  using raw_value_type = base::remove_pointer<T>::type;
  using allocator_type = TAllocator;
  using value_reference = T&;
  using const_value_reference = const T&;

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
  Vector(std::initializer_list<value_type> list) {
    const auto count = list.size();
    data_ = Vector::Allocate(count);
    capacity_ = data_ + count;
    end_ = data_;  // Start at the beginning
    T* current = data_;
    for (const auto& item : list) {
      ::new (static_cast<void*>(current++)) T(item);
    }
    end_ = current;  // Set end_ to its final position
  }

  // move constructor
  Vector(Vector&& other) noexcept {
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

  // move assignment operator
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

  void resize(mem_size new_capacity, const value_type& value) {
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
    BASE_DCHECK(new_reserved_capacity != 0 && capacity() != 0,
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

  void push_back(const value_type& value) {
    // cram it into pre over reserved space
    if (end_ < capacity_) [[likely]]
      ::new (static_cast<void*>(end_++)) value_type(value);
    else
      InsertAtEnd(value);
  }

  void push_back(value_type&& value) {
    if (end_ < capacity_) [[likely]]
      ::new (static_cast<void*>(end_++)) value_type(base::move(value));
    else
      InsertAtEnd(base::move(value));
  }

  template <typename... TArgs>
  value_type& emplace_back(TArgs&&... args) {
    if (end_ < capacity_) [[likely]]
      ::new (static_cast<void*>(end_++)) value_type(base::forward<TArgs>(args)...);
    else
      InsertAtEnd(base::forward<TArgs>(args)...);
    return back();
  }

  [[nodiscard]] constexpr raw_value_type* at(mem_size pos) const {
    if (pos >= size()) {
      return nullptr;
    }
    if constexpr (base::is_pointer<value_type>::value) {
      return data_[pos];
    } else {
      return &data_[pos];
    }
  }

  [[nodiscard]] T* find(const T& element_match) {
    for (auto* it = begin(); it != end(); ++it) {
      if (*it == element_match) {
        return it;
      }
    }
    return nullptr;
  }

  [[nodiscard]] const T* find(const T& element_match) const {
    for (const auto* it = begin(); it != end(); ++it) {
      if (*it == element_match) {
        return it;
      }
    }
    return nullptr;
  }

  [[nodiscard]] bool Contains(const T& element_match) const {
    return find(element_match) != nullptr;
  }

  // single element at a specified position.
  T* insert(T* pos, const T& value) {
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    if (size() == capacity()) {
      const mem_size new_cap = CalculateNewCapacity(size());
      GrowCapacity(size(), new_cap);
      pos = begin() + index;  // Recalculate iterator after growth
    }

    MakeHoleForInsert(pos, 1);

    // Safely place the new value.
    if (pos < end_) {
      *pos = value;  // Assign into the now-vacant (moved-from) spot.
    } else {
      ::new (static_cast<void*>(pos)) T(value);  // Construct at the end.
    }

    end_++;
    return begin() + index;
  }

  // Inserts multiple copies of an element.
  void insert(T* pos, size_t count, const T& value) {
    if (count == 0)
      return;
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    if (size() + count > capacity()) {
      const mem_size new_cap = CalculateNewCapacity(size() + count);
      GrowCapacity(size(), new_cap);
      pos = begin() + index;  // Recalculate iterator
    }

    MakeHoleForInsert(pos, count);

    // Fill the hole with the new value.
    for (size_t i = 0; i < count; ++i) {
      ::new (static_cast<void*>(pos + i)) T(value);  // Always safe to construct here
    }

    end_ += count;
  }

  // Inserts a range of elements.
  template <class InputIt>
  void insert(T* pos, InputIt first, InputIt last) {
    // Manually calculate distance
    mem_size count = 0;
    for (InputIt it = first; it != last; ++it) {
      count++;
    }

    if (count == 0)
      return;
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    if (size() + count > capacity()) {
      const mem_size new_cap = CalculateNewCapacity(size() + count);
      GrowCapacity(size(), new_cap);
      pos = begin() + index;  // Recalculate iterator
    }

    MakeHoleForInsert(pos, count);

    // Fill the hole from the source range.
    T* dest = pos;
    for (InputIt it = first; it != last; ++it, ++dest) {
      ::new (static_cast<void*>(dest)) T(*it);
    }

    end_ += count;
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

    // if we remove in the middle, we memmove the upper objects down by one
    // place.
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
    BASE_DCHECK(!empty(), "Vector is empty.");
    return data_[0];
  }

  // Access first element (const)
  const T& front() const {
    BASE_DCHECK(!empty(), "Vector is empty.");
    return data_[0];
  }

  [[nodiscard]] const T& back() const {
    BASE_DCHECK(!empty());
    return *(end_ - 1);
  }

  [[nodiscard]] T& back() {
    BASE_DCHECK(!empty());
    return *(end_ - 1);
  }

  [[nodiscard]] T* begin() const { return data_; }
  [[nodiscard]] T* end() const { return end_; }
  [[nodiscard]] T* data() { return data_; }
  [[nodiscard]] const T* data() const { return data_; }

  [[nodiscard]] bool empty() const { return data_ == nullptr || end_ == data_; }
  [[nodiscard]] mem_size size() const { return end_ - data_; }
  [[nodiscard]] mem_size capacity() const { return capacity_ - data_; }

  [[nodiscard]] BASE_CONSTEXPR_ND T& operator[](mem_size pos) const {
    BASE_DCHECK(pos <= size(), "Vector::[]: Access out of bounds");
    return data_[pos];
  }

  template <typename TFunc>
  void ForEach(TFunc&& func) {
    for (auto* it = begin(); it != end(); ++it) {
      func(*it);
    }
  }

  template <typename TFunc>
  T* FindIf(TFunc&& func) {
    for (auto* it = begin(); it != end(); ++it) {
      if (func(*it))
        return it;
    }
    return nullptr;
  }

 private:
  mem_size CalculateNewCapacity(mem_size cap) {
    return cap > 0 ? cap * /*capacity_mult_*/ kDefaultMult : 1;
  }

  void MakeHoleForInsert(T* pos, mem_size count) {
    const mem_size elements_to_move = end_ - pos;
    if (elements_to_move == 0 || count == 0) {
      return;  // Nothing to shift.
    }

    T* const old_end = end_;

    // Elements being shifted into what was previously beyond the vector's end
    // must be move-constructed into uninitialized memory.
    const mem_size num_to_construct = base::Min(count, elements_to_move);
    for (mem_size i = 0; i < num_to_construct; ++i) {
      T* source = old_end - (i + 1);
      T* dest = source + count;
      ::new (static_cast<void*>(dest)) T(base::move(*source));
    }

    // Elements being shifted into memory that was already occupied by other
    // elements can be safely move-assigned.
    for (mem_size i = num_to_construct; i < elements_to_move; ++i) {
      T* source = old_end - (i + 1);
      T* dest = source + count;
      *dest = base::move(*source);
    }
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
      const auto new_size = grow_size < current_cap + n ? current_cap + n : grow_size;

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
      const auto new_size = grow_size < current_cap + n ? current_cap + n : grow_size;

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

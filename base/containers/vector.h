// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Vector is a sequence container that encapsulates dynamic size arrays
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/allocation_size.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>
#include <base/math/value_bounds.h>
#include <base/containers/container_traits.h>

#include <new>      // < for placement new
#include <cstring>  // < linux memory stuff
#include <initializer_list>

namespace base {

enum class VectorReservePolicy {
  kForPushback,
  kForData,
};

template <typename T, class TAllocator = base::DefaultAllocator>
class Vector {
 public:
  using value_type = T;
  using raw_value_type = typename base::remove_pointer<T>::type;
  using allocator_type = TAllocator;
  using value_reference = T&;
  using const_value_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;
  // Names the standard container adaptors (priority_queue, stack, queue) look
  // up on their backing container.
  using size_type = mem_size;
  using difference_type = pointer_diff;
  using reference = T&;
  using const_reference = const T&;

  constexpr static mem_size kDefaultMult = 2;

  Vector() : data_(nullptr), end_(nullptr), capacity_(nullptr) {}

  // Size constructor: creates a vector of `count` default-constructed elements.
  explicit Vector(mem_size count) : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    if (count > 0) {
      data_ = Vector::Allocate(count);
      capacity_ = &data_[count];
      T* current = data_;
      for (mem_size i = 0; i < count; ++i, ++current) {
        ::new (static_cast<void*>(current)) T();
      }
      end_ = capacity_;
    }
  }

  // Fill constructor: `count` copies of `value`.
  Vector(mem_size count, const T& value) : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    if (count > 0) {
      data_ = Vector::Allocate(count);
      capacity_ = &data_[count];
      T* current = data_;
      for (mem_size i = 0; i < count; ++i, ++current) {
        ::new (static_cast<void*>(current)) T(value);
      }
      end_ = capacity_;
    }
  }

  // Range constructor: copies [first, last). Single-pass safe, so it also takes
  // input iterators whose range length is not known up front.
  // Constrained on dereferenceability so `Vector<int> v(5, 3)` still picks the
  // fill constructor rather than treating the two ints as a range.
  template <typename InputIt>
    requires requires(InputIt it) {
      *it;
      ++it;
    }
  Vector(InputIt first, InputIt last) : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  Vector(mem_size reserve_count, const VectorReservePolicy policy)
      : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    if (reserve_count > 0) {
      data_ = Vector::Allocate(reserve_count);
      capacity_ = &data_[reserve_count];
      if (policy == VectorReservePolicy::kForData) {
        if constexpr (__is_constructible(T)) {
          T* current = data_;
          for (mem_size i = 0; i < reserve_count; ++i, ++current) {
            ::new (static_cast<void*>(current)) T();
          }
          end_ = capacity_;
        } else {
          BASE_BUGCHECK(false, "kForData requires default-constructible T");
        }
      } else {
        end_ = data_;
      }
    }
  }

  // from braces {}
  Vector(std::initializer_list<value_type> list)
      : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    const auto count = list.size();
    if (count > 0) {
      data_ = Vector::Allocate(count);
      capacity_ = data_ + count;
      T* current = data_;
      for (const auto& item : list) {
        ::new (static_cast<void*>(current++)) T(item);
      }
      end_ = current;
    }
  }

  // copy constructor
  Vector(const Vector& other) : data_(nullptr), end_(nullptr), capacity_(nullptr) {
    const auto count = other.size();
    if (count > 0) {
      data_ = Vector::Allocate(count);
      capacity_ = &data_[count];
      T* dest = data_;
      const T* src = other.data_;
      for (mem_size i = 0; i < count; ++i, ++src, ++dest) {
        ::new (static_cast<void*>(dest)) T(*src);
      }
      end_ = dest;
    }
  }

  // copy assignment operator
  Vector& operator=(const Vector& other) {
    if (this != &other) {
      base::DestructRange(data_, end_);
      Free(data_, capacity());
      data_ = nullptr;
      end_ = nullptr;
      capacity_ = nullptr;

      const auto count = other.size();
      if (count > 0) {
        data_ = Vector::Allocate(count);
        capacity_ = &data_[count];
        T* dest = data_;
        const T* src = other.data_;
        for (mem_size i = 0; i < count; ++i, ++src, ++dest) {
          ::new (static_cast<void*>(dest)) T(*src);
        }
        end_ = dest;
      }
    }
    return *this;
  }

  // move constructor
  Vector(Vector&& other) noexcept
      : data_(other.data_), end_(other.end_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.end_ = nullptr;
    other.capacity_ = nullptr;
  }

  ~Vector() {
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

  void resize(mem_size new_size, const value_type& value) {
    const auto current_size = size();
    if (new_size > current_size) {
      const auto count = new_size - current_size;
      if (new_size > capacity()) {
        GrowCapacity(current_size, new_size);
      }
      T* ptr = data_ + current_size;
      for (mem_size i = 0; i < count; ++i) {
        ::new (static_cast<void*>(ptr++)) T(value);
      }
      end_ = data_ + new_size;
    } else if (new_size < current_size) {
      base::DestructRange(data_ + new_size, end_);
      end_ = data_ + new_size;
    }
  }

  void resize(mem_size new_size) {
    const auto current_size = size();
    if (new_size > current_size) {
      const auto count = new_size - current_size;
      if (new_size > capacity()) {
        GrowCapacity(current_size, new_size);
      }
      T* ptr = data_ + current_size;
      for (mem_size i = 0; i < count; ++i) {
        ::new (static_cast<void*>(ptr++)) T();
      }
      end_ = data_ + new_size;
    } else if (new_size < current_size) {
      base::DestructRange(data_ + new_size, end_);
      end_ = data_ + new_size;
    }
  }

  // increase internal capacity (does not change size)
  void reserve(mem_size new_reserved_capacity) {
    if (new_reserved_capacity == 0)
      return;
    if (new_reserved_capacity > capacity())
      GrowCapacity(size(), new_reserved_capacity);
  }

  mem_size shrink_to_fit() {
    if (end_ != capacity_) [[likely]] {
      if (data_ == end_) {
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

  void pop_back() {
    BASE_DCHECK(!empty(), "Vector::pop_back: empty vector");
    --end_;
    end_->~T();
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
      if (*it == element_match)
        return it;
    }
    return nullptr;
  }

  [[nodiscard]] const T* find(const T& element_match) const {
    for (const auto* it = begin(); it != end(); ++it) {
      if (*it == element_match)
        return it;
    }
    return nullptr;
  }

  [[nodiscard]] bool Contains(const T& element_match) const {
    return find(element_match) != nullptr;
  }

  // single element at a specified position
  T* insert(T* pos, const T& value) {
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    if (size() == capacity()) {
      const mem_size new_cap = CalculateNewCapacity(size());
      GrowCapacity(size(), new_cap);
      pos = begin() + index;
    }

    MakeHoleForInsert(pos, 1);

    if (pos < end_) {
      *pos = value;
    } else {
      ::new (static_cast<void*>(pos)) T(value);
    }

    end_++;
    return begin() + index;
  }

  // Inserts multiple copies of an element
  void insert(T* pos, mem_size count, const T& value) {
    if (count == 0)
      return;
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    const mem_size required = base::CheckedCountSum(size(), count);
    if (required > capacity()) {
      const mem_size new_cap = CalculateNewCapacity(required);
      GrowCapacity(size(), new_cap);
      pos = begin() + index;
    }

    const mem_size elements_after = end_ - pos;
    MakeHoleForInsert(pos, count);

    // Positions [pos..pos+min(count,elements_after)-1] contain moved-from but
    // alive objects; use assignment. Positions past old end_ need placement new.
    const mem_size num_assign = base::Min(count, elements_after);
    for (mem_size i = 0; i < num_assign; ++i) {
      *(pos + i) = value;
    }
    for (mem_size i = num_assign; i < count; ++i) {
      ::new (static_cast<void*>(pos + i)) T(value);
    }

    end_ += count;
  }

  // Inserts a range of elements
  void insert(T* pos, std::initializer_list<T> values) {
    insert(pos, values.begin(), values.end());
  }

  template <class InputIt>
  void insert(T* pos, InputIt first, InputIt last) {
    mem_size count = 0;
    for (InputIt it = first; it != last; ++it) {
      count++;
    }

    if (count == 0)
      return;
    BASE_DCHECK(pos >= begin() && pos <= end(), "Vector::insert: Invalid position");
    const auto index = pos - begin();

    const mem_size required = base::CheckedCountSum(size(), count);
    if (required > capacity()) {
      const mem_size new_cap = CalculateNewCapacity(required);
      GrowCapacity(size(), new_cap);
      pos = begin() + index;
    }

    const mem_size elements_after = end_ - pos;
    MakeHoleForInsert(pos, count);

    // Positions in the old range contain moved-from but alive objects.
    const mem_size num_assign = base::Min(count, elements_after);
    T* dest = pos;
    mem_size i = 0;
    for (InputIt it = first; it != last; ++it, ++dest, ++i) {
      if (i < num_assign) {
        *dest = *it;
      } else {
        ::new (static_cast<void*>(dest)) T(*it);
      }
    }

    end_ += count;
  }

  // erase by index - properly handles non-trivial types
  bool erase(mem_size pos) {
    if (pos >= size())
      return false;

    T* dest = &data_[pos];

    // Move-assign all elements after pos down by one
    for (T* it = dest + 1; it != end_; ++it) {
      *(it - 1) = base::move(*it);
    }

    --end_;
    end_->~T();
    return true;
  }

  // erase by pointer
  bool erase(T* element_ptr) {
    if (element_ptr < data_ || element_ptr >= end_)
      return false;

    for (T* it = element_ptr + 1; it != end_; ++it) {
      *(it - 1) = base::move(*it);
    }

    --end_;
    end_->~T();
    return true;
  }

  // erase range [first, last)
  T* erase(T* first, T* last) {
    BASE_DCHECK(first >= begin() && first <= end(), "Vector::erase: invalid first");
    BASE_DCHECK(last >= first && last <= end(), "Vector::erase: invalid last");

    if (first == last)
      return first;

    // Move elements after the range down
    T* dest = first;
    for (T* src = last; src != end_; ++src, ++dest) {
      *dest = base::move(*src);
    }

    // Destroy trailing elements
    base::DestructRange(dest, end_);
    end_ = dest;
    return first;
  }

  void clear() noexcept {
    base::DestructRange(data_, end_);
    end_ = data_;
  }

  void assign(mem_size count, const T& value) {
    clear();
    resize(count, value);
  }

  // Constrained on dereferenceability, so `assign(count, value)` with an
  // integral count is not swallowed by the range overload.
  template <typename InputIt>
    requires requires(InputIt it) {
      *it;
      ++it;
    }
  void assign(InputIt first, InputIt last) {
    clear();
    for (; first != last; ++first)
      push_back(*first);
  }

  void reset() {
    base::DestructRange(data_, end_);
    Vector::Free(data_, capacity());
    data_ = nullptr;
    end_ = nullptr;
    capacity_ = nullptr;
  }

  // Constant-time ownership exchange; no element is moved.
  void swap(Vector& other) noexcept {
    T* const data = data_;
    T* const end = end_;
    T* const capacity = capacity_;
    data_ = other.data_;
    end_ = other.end_;
    capacity_ = other.capacity_;
    other.data_ = data;
    other.end_ = end;
    other.capacity_ = capacity;
  }

  T& front() {
    BASE_DCHECK(!empty(), "Vector is empty.");
    return data_[0];
  }

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

  // Reverse traversal. ReverseIterator walks backwards over the same storage,
  // so `for (auto it = v.rbegin(); it != v.rend(); ++it)` reads last-to-first.
  class ReverseIterator {
   public:
    explicit ReverseIterator(T* at) : at_(at) {}

    bool operator==(const ReverseIterator& other) const { return at_ == other.at_; }
    bool operator!=(const ReverseIterator& other) const { return at_ != other.at_; }

    ReverseIterator& operator++() {
      --at_;
      return *this;
    }

    T& operator*() const { return *(at_ - 1); }
    T* operator->() const { return at_ - 1; }

   private:
    T* at_;
  };

  [[nodiscard]] ReverseIterator rbegin() const { return ReverseIterator(end_); }
  [[nodiscard]] ReverseIterator rend() const { return ReverseIterator(data_); }

  [[nodiscard]] T* data() { return data_; }
  [[nodiscard]] const T* data() const { return data_; }

  [[nodiscard]] bool empty() const { return data_ == nullptr || end_ == data_; }
  [[nodiscard]] mem_size size() const { return end_ - data_; }
  [[nodiscard]] mem_size capacity() const { return capacity_ - data_; }

  [[nodiscard]] BASE_CONSTEXPR_ND T& operator[](mem_size pos) const {
    BASE_DCHECK(pos < size(), "Vector::[]: Access out of bounds");
    return data_[pos];
  }

  bool operator==(const Vector& other) const {
    if (size() != other.size())
      return false;
    for (mem_size i = 0; i < size(); ++i) {
      if (!(data_[i] == other.data_[i]))
        return false;
    }
    return true;
  }

  bool operator!=(const Vector& other) const { return !(*this == other); }

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
    return cap > 0 ? base::CheckedCountProduct(cap, kDefaultMult) : 1;
  }

  void MakeHoleForInsert(T* pos, mem_size count) {
    const mem_size elements_to_move = end_ - pos;
    if (elements_to_move == 0 || count == 0)
      return;

    T* const old_end = end_;

    const mem_size num_to_construct = base::Min(count, elements_to_move);
    for (mem_size i = 0; i < num_to_construct; ++i) {
      T* source = old_end - (i + 1);
      T* dest = source + count;
      ::new (static_cast<void*>(dest)) T(base::move(*source));
    }

    for (mem_size i = num_to_construct; i < elements_to_move; ++i) {
      T* source = old_end - (i + 1);
      T* dest = source + count;
      *dest = base::move(*source);
    }
  }

  template <typename... TArgs>
  void InsertAtEnd(TArgs&&... args) {
    const auto current_size = size();
    const auto new_cap = CalculateNewCapacity(current_size);
    GrowCapacity(current_size, new_cap);
    ::new (static_cast<void*>(end_++)) T(base::forward<TArgs>(args)...);
  }

  void GrowCapacity(mem_size current_size, mem_size new_cap) {
    T* new_block = Vector::Allocate(new_cap);

    if (data_) {
      auto* first = data_;
      auto* last = end_;
      auto* new_spot = new_block;
      for (; first != last; ++first, ++new_spot) {
        ::new (reinterpret_cast<void*>(new_spot)) T(base::move(*first));
      }
      base::DestructRange(data_, end_);
      Vector::Free(data_, capacity());
    }

    data_ = new_block;
    end_ = &new_block[current_size];
    capacity_ = &new_block[new_cap];
  }

  void ReleaseAll() {
    if (data_) {
      base::DestructRange(data_, end_);
      Vector::Free(data_, capacity());
      data_ = end_ = capacity_ = nullptr;
    }
  }

  T* Allocate(mem_size cap) {
    auto* block =
        static_cast<T*>(TAllocator::Allocate(base::CheckedAllocationSize(cap, sizeof(T))));
    // DefaultAllocator throws instead of returning null, but TAllocator is a
    // template parameter and a custom one may report failure by returning it.
    // Every caller constructs elements into the block straight away, so a null
    // block has to stop here rather than become an out-of-bounds write.
    BASE_FATAL_CHECK(block, "Vector: allocation failed");
    return block;
  }
  void Free(T* block, mem_size n) {
    if (block)
      TAllocator::Free(reinterpret_cast<void*>(block), n * sizeof(T));
  }

 private:
  T* data_;
  T* end_;
  T* capacity_;
};
}  // namespace base

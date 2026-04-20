// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// BasicFixedString<N, TChar>
//
// A pure-stack string with a hard upper bound of N characters. Never touches
// the heap. Trivially copyable, so it lives happily inside POD components,
// network packets, ECS data, hash keys, etc. Operations that would overflow
// trip a BASE_BUGCHECK rather than allocating.
//
// Layout: TChar data_[N + 1] (always null-terminated) + u32 size_.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/meta/traits.h>
#include <base/numeric_limits.h>
#include <base/strings/char_algorithms.h>

#include <cstring>

namespace base {

template <mem_size N, typename TChar = char>
class BasicFixedString {
  static_assert(N > 0, "BasicFixedString requires N > 0");
  static_assert(base::is_trivial_v<TChar>, "TChar must be trivial");

 public:
  using character_type = TChar;
  using value_type = character_type;
  using size_type = u32;
  static constexpr size_type npos = base::MinMax<size_type>::max();
  static constexpr size_type kCapacity = static_cast<size_type>(N);

  // Defaulted special members keep this trivially copyable.
  BasicFixedString() = default;
  BasicFixedString(const BasicFixedString&) = default;
  BasicFixedString(BasicFixedString&&) = default;
  BasicFixedString& operator=(const BasicFixedString&) = default;
  BasicFixedString& operator=(BasicFixedString&&) = default;
  ~BasicFixedString() = default;

  BasicFixedString(const character_type* str) { assign(str); }

  BasicFixedString(const character_type* str, size_type len) {
    assign(str, len);
  }

  BasicFixedString(const character_type* begin, const character_type* end) {
    assign(begin, static_cast<size_type>(end - begin));
  }

  BasicFixedString(size_type count, character_type c) {
    BASE_BUGCHECK(count <= kCapacity, "FixedString overflow in fill ctor");
    for (size_type i = 0; i < count; ++i) data_[i] = c;
    size_ = count;
    data_[size_] = character_type{};
  }

  template <size_type ArraySize>
  BasicFixedString(const character_type (&arr)[ArraySize]) {
    assign(arr,
           static_cast<size_type>(
               base::CountStringLength(arr, ArraySize > 0 ? ArraySize - 1 : 0)));
  }

  // -- Assignment --

  BasicFixedString& operator=(const character_type* str) {
    assign(str);
    return *this;
  }

  void assign(const character_type* str, size_type len) {
    BASE_BUGCHECK(len <= kCapacity, "FixedString overflow in assign");
    if (str && len > 0) {
      memcpy(data_, str, len * sizeof(character_type));
    }
    size_ = len;
    data_[size_] = character_type{};
  }
  void assign(const character_type* str) {
    assign(str, static_cast<size_type>(base::CountStringLength(str)));
  }
  void assign(const character_type* start, const character_type* end) {
    assign(start, static_cast<size_type>(end - start));
  }

  // -- Access --

  const character_type* c_str() const noexcept { return data_; }
  character_type* data() noexcept { return data_; }
  const character_type* data() const noexcept { return data_; }

  character_type* begin() noexcept { return data_; }
  character_type* end() noexcept { return data_ + size_; }
  const character_type* begin() const noexcept { return data_; }
  const character_type* end() const noexcept { return data_ + size_; }

  character_type& operator[](size_type i) {
    BASE_BUGCHECK(i < size_, "Index out of bounds");
    return data_[i];
  }
  const character_type& operator[](size_type i) const {
    BASE_BUGCHECK(i < size_, "Index out of bounds");
    return data_[i];
  }
  character_type& at(size_type i) { return (*this)[i]; }
  const character_type& at(size_type i) const { return (*this)[i]; }

  character_type& back() {
    BASE_BUGCHECK(size_ > 0, "back() on empty string");
    return data_[size_ - 1];
  }
  const character_type& back() const {
    BASE_BUGCHECK(size_ > 0, "back() on empty string");
    return data_[size_ - 1];
  }

  // -- Capacity --

  size_type size() const noexcept { return size_; }
  size_type length() const noexcept { return size_; }
  size_type byte_size() const noexcept { return size_ * sizeof(character_type); }
  bool empty() const noexcept { return size_ == 0; }
  static constexpr size_type capacity() noexcept { return kCapacity; }
  static constexpr size_type max_size() noexcept { return kCapacity; }

  void clear() noexcept {
    size_ = 0;
    data_[0] = character_type{};
  }

  void resize(size_type new_size) {
    BASE_BUGCHECK(new_size <= kCapacity, "FixedString overflow in resize");
    if (new_size > size_) {
      memset(data_ + size_, 0, (new_size - size_) * sizeof(character_type));
    }
    size_ = new_size;
    data_[size_] = character_type{};
  }

  // -- Modification --

  BasicFixedString& operator+=(const character_type* str) {
    append(str);
    return *this;
  }
  BasicFixedString& operator+=(character_type c) {
    push_back(c);
    return *this;
  }
  BasicFixedString& operator+=(const BasicFixedString& other) {
    append(other.data_, other.size_);
    return *this;
  }

  void append(const character_type* str, size_type count) {
    BASE_BUGCHECK(size_ + count <= kCapacity,
                  "FixedString overflow in append");
    if (count > 0) {
      memcpy(data_ + size_, str, count * sizeof(character_type));
      size_ += count;
      data_[size_] = character_type{};
    }
  }
  void append(const character_type* str) {
    append(str, static_cast<size_type>(base::CountStringLength(str)));
  }

  void push_back(character_type c) {
    BASE_BUGCHECK(size_ < kCapacity, "FixedString overflow in push_back");
    data_[size_++] = c;
    data_[size_] = character_type{};
  }

  // Best-effort: copy at most kCapacity chars from str. Useful when truncation
  // is acceptable (logging, debug labels). Returns true if the input fit.
  bool assign_truncating(const character_type* str) {
    if (!str) {
      clear();
      return true;
    }
    const mem_size full = base::CountStringLength(str);
    const size_type to_copy =
        full > kCapacity ? kCapacity : static_cast<size_type>(full);
    memcpy(data_, str, to_copy * sizeof(character_type));
    size_ = to_copy;
    data_[size_] = character_type{};
    return full <= kCapacity;
  }

  // -- Compare --

  int compare(const BasicFixedString& other) const noexcept {
    return compare(other.data_, other.size_);
  }
  int compare(const character_type* str) const noexcept {
    return compare(str, static_cast<size_type>(base::CountStringLength(str)));
  }
  int compare(const character_type* str, size_type count) const noexcept {
    const size_type m = size_ < count ? size_ : count;
    int r = memcmp(data_, str, m * sizeof(character_type));
    if (r != 0) return r;
    if (size_ < count) return -1;
    if (size_ > count) return 1;
    return 0;
  }

  // -- Search --

  size_type find(character_type c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    for (size_type i = pos; i < size_; ++i) {
      if (data_[i] == c) return i;
    }
    return npos;
  }

 private:
  // In-class init zero-fills both members so a default-constructed instance
  // is a valid empty string. The defaulted copy/move/dtor remain trivial,
  // so the type stays trivially copyable (memcpy-safe).
  character_type data_[N + 1] = {};
  size_type size_ = 0;
};

// -- Non-member comparison --

template <mem_size N, typename TChar>
bool operator==(const BasicFixedString<N, TChar>& lhs,
                const BasicFixedString<N, TChar>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  return memcmp(lhs.data(), rhs.data(), lhs.byte_size()) == 0;
}

template <mem_size N, typename TChar>
bool operator==(const BasicFixedString<N, TChar>& lhs, const TChar* rhs) {
  if (rhs == nullptr) return lhs.empty();
  if (base::CountStringLength(rhs) != lhs.size()) return false;
  return memcmp(lhs.data(), rhs, lhs.byte_size()) == 0;
}

template <mem_size N, typename TChar>
bool operator!=(const BasicFixedString<N, TChar>& lhs,
                const BasicFixedString<N, TChar>& rhs) {
  return !(lhs == rhs);
}

template <mem_size N, typename TChar>
bool operator<(const BasicFixedString<N, TChar>& lhs,
               const BasicFixedString<N, TChar>& rhs) {
  return lhs.compare(rhs) < 0;
}

}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Span utility for peeking into abstract data object with a common interface.

#pragma once

#include <base/check.h>

namespace base {

// Span is a trivial data type that we guarantee has ABI compatability with our
// interfaces.
template <typename T>
class Span {
 public:
  explicit Span(T* ptr, size_t len) : ptr_(ptr), len_(len) {}

  template <size_t N>
  constexpr Span(T (&a)[N]) noexcept  // NOLINT(runtime/explicit)
      : Span(a, N) {}

  T* data() const { return ptr_; }
  size_t size() const { return len_; }
  size_t length() const { return len_; }
  bool empty() const { return ptr_; }

  constexpr T& operator[](size_t index) const {
    // DCHECK(i < len_);
    return ptr_[index];
  }

  constexpr T& front() {
    DCHECK(ptr_ && len_ > 0);
    return *ptr_;
  }

  constexpr T& back() {
    DCHECK(ptr_ && len_ > 0);
    return *ptr_[len_ - 1];
  }

 private:
  T* ptr_;
  size_t len_;
};

// static_assert(sizeof(Span<void>) == 16);
}  // namespace base
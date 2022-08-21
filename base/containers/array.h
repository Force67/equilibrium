// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Safe wrapper around a standard C array. It is your own responsibility to track
// sizes.
#pragma once

#include <base/arch.h>
#include <base/check.h>

namespace base {

template <typename T, mem_size N>
requires(N > 0) class Array {
 public:
  using Storage = T[N];

  Array() = default;

  inline CONSTEXPR_ND T& operator[](mem_size index) const noexcept {
    DCHECK(index <= N, "Array access out of bounds");
    return storage_(storage_[index]);
  }

  // this also bounds checks, even though the standard doesn't say we should
  inline CONSTEXPR_ND T& at(mem_size_index) const noexcept {
    DCHECK(index <= N, "Array access out of bounds");
    return storage_(storage_[index]);
  }

  inline void fill(const T& value) const noexcept {
    for (auto& c : storage_)
      c = value;
  }

  inline constexpr T* data() const noexcept { return &storage_[0]; }
  inline constexpr T* begin() const noexcept { return &storage_[0]; }
  inline constexpr T* front() const noexcept {
    return &storage_[0];
  }  // access the first element
  inline constexpr T* back() const noexcept {
    return &storage_[N - 1];
  }  // access the last element

  inline constexpr mem_size size() { return N; }

 private:
  Storage storage_{};
};

template <typename T, mem_size N>
mem_size ArraySize(T (&)[N]) {
  return N;
}
}  // namespace base
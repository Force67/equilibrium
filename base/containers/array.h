// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Safe wrapper around a standard C array.
#pragma once

#include <base/arch.h>
#include <base/check.h>

namespace base {

template <typename T, mem_size N>
  requires(N > 0)
class Array {
 public:
  using Storage = T[N];

  Array() = default;

  inline BASE_CONSTEXPR_ND T& operator[](mem_size index) noexcept {
    BASE_DCHECK(index < N, "Array access out of bounds");
    return storage_[index];
  }

  inline BASE_CONSTEXPR_ND const T& operator[](mem_size index) const noexcept {
    BASE_DCHECK(index < N, "Array access out of bounds");
    return storage_[index];
  }

  inline BASE_CONSTEXPR_ND T& at(mem_size index) noexcept {
    BASE_DCHECK(index < N, "Array access out of bounds");
    return storage_[index];
  }

  inline BASE_CONSTEXPR_ND const T& at(mem_size index) const noexcept {
    BASE_DCHECK(index < N, "Array access out of bounds");
    return storage_[index];
  }

  inline void fill(const T& value) noexcept {
    for (auto& c : storage_)
      c = value;
  }

  inline constexpr T* data() noexcept { return &storage_[0]; }
  inline constexpr const T* data() const noexcept { return &storage_[0]; }

  inline constexpr T* begin() noexcept { return &storage_[0]; }
  inline constexpr const T* begin() const noexcept { return &storage_[0]; }
  inline constexpr T* end() noexcept { return &storage_[N]; }
  inline constexpr const T* end() const noexcept { return &storage_[N]; }

  inline constexpr T& front() noexcept { return storage_[0]; }
  inline constexpr const T& front() const noexcept { return storage_[0]; }
  inline constexpr T& back() noexcept { return storage_[N - 1]; }
  inline constexpr const T& back() const noexcept { return storage_[N - 1]; }

  inline constexpr mem_size size() const { return N; }
  inline constexpr bool empty() const { return false; }

 private:
  Storage storage_{};
};

template <typename T, mem_size N>
mem_size ArraySize(T (&)[N]) {
  return N;
}
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Safe wrapper around a standard C array.
#pragma once

#include <initializer_list>

#include <base/arch.h>
#include <base/check.h>

namespace base {

template <typename T, mem_size N>
  requires(N > 0)
class Array {
 public:
  using Storage = T[N];

  Array() = default;

  // Brace initialization, like a raw array: `Array<f32, 3> v{1, 2, 3}`. Fewer
  // elements than N leaves the tail value-initialized; more is a bug.
  constexpr Array(std::initializer_list<T> values) {
    BASE_DCHECK(values.size() <= N, "Array initializer list longer than the array");
    mem_size i = 0;
    for (const T& value : values) {
      if (i >= N) break;
      storage_[i++] = value;
    }
  }

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

  constexpr bool operator==(const Array& other) const {
    for (mem_size i = 0; i < N; ++i) {
      if (!(storage_[i] == other.storage_[i])) return false;
    }
    return true;
  }

  constexpr bool operator!=(const Array& other) const { return !(*this == other); }

 private:
  Storage storage_{};
};

template <typename T, mem_size N>
mem_size ArraySize(T (&)[N]) {
  return N;
}
}  // namespace base

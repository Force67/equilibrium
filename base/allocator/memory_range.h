// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

template <typename T = mem_size>
class MemoryRange {
 public:
  using size_type = T;

  inline constexpr explicit MemoryRange(void* ptr)
      : start_(reinterpret_cast<pointer_size>(ptr)), size_(0) {}

  inline constexpr MemoryRange(void* ptr, size_type size)
      : start_(reinterpret_cast<pointer_size>(ptr)), size_(size) {}

  constexpr bool Contains(pointer_size start, size_type size) const noexcept {
    return (start >= start_) && ((start + size) <= (start_ + size_));
  }

  constexpr bool Contains(pointer_size address) const noexcept {
    return (address >= start_) && (address < (start_ + size_));
  }

 private:
  pointer_size start_{0};
  size_type size_;
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/numeric_limits.h>

namespace base {

template <typename T>
[[nodiscard]] mem_size PopCount(const T value) noexcept {
  constexpr mem_size _Digits = base::MinMax<T>::digits();
  if constexpr (_Digits <= 16) {
    return static_cast<mem_size>(__popcnt16(value));
  } else if constexpr (_Digits == 32) {
    return static_cast<mem_size>(__popcnt(value));
  } else {
#ifdef _M_IX86
    return static_cast<mem_size>(__popcnt(value >> 32) +
                                 __popcnt(static_cast<unsigned int>(value)));
#else   // ^^^ _M_IX86 / !_M_IX86 vvv
    return static_cast<mem_size>(__popcnt64(value));
#endif  // _M_IX86
  }
}
}
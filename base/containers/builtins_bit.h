// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/meta/traits.h>
#include <base/numeric_limits.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace base {

#if defined(_MSC_VER)
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
#endif

#if defined(OS_POSIX)
template <typename T>
[[nodiscard]] mem_size PopCount(const T value) noexcept {
  constexpr mem_size _Digits = base::MinMax<T>::digits();
  if constexpr (_Digits <= 16) {
    return static_cast<mem_size>(__builtin_popcount(static_cast<u32>(value)));
  } else if constexpr (_Digits == 32) {
    return static_cast<mem_size>(__builtin_popcount(value));
  } else {
#ifdef _M_IX86
    return static_cast<mem_size>(__builtin_popcount(value >> 32) +
                                 __builtin_popcount(static_cast<unsigned int>(value)));
#else   // ^^^ _M_IX86 / !_M_IX86 vvv
    return static_cast<mem_size>(__builtin_popcountll(value));
#endif  // _M_IX86
  }
}
#endif

// Count of leading zero bits in |value|, as std::countl_zero would give. The
// builtins are undefined for zero, so that case is answered from the width.
template <typename T>
[[nodiscard]] constexpr int CountLeftZero(const T value) noexcept {
  // Width from sizeof, not MinMax<T>::digits(): digits() follows
  // std::numeric_limits and drops the sign bit, which is one short of the
  // bit count these builtins actually operate on.
  static_assert(base::is_integral_v<T> && !base::is_signed_v<T>,
                "bit counting is defined for unsigned integers only");
  constexpr int kDigits = static_cast<int>(sizeof(T) * 8);
  if (value == 0)
    return kDigits;
#if defined(_MSC_VER) && !defined(__clang__)
  unsigned long index = 0;
  if constexpr (kDigits <= 32) {
    ::_BitScanReverse(&index, static_cast<unsigned long>(value));
    return kDigits - 1 - static_cast<int>(index);
  } else {
    ::_BitScanReverse64(&index, static_cast<unsigned long long>(value));
    return kDigits - 1 - static_cast<int>(index);
  }
#else
  if constexpr (kDigits <= 32) {
    // __builtin_clz works on a 32-bit unsigned; narrower types are widened,
    // so subtract the padding the widening introduced.
    return __builtin_clz(static_cast<unsigned int>(value)) - (32 - kDigits);
  } else {
    return __builtin_clzll(static_cast<unsigned long long>(value));
  }
#endif
}

// Count of trailing zero bits in |value|, as std::countr_zero would give.
template <typename T>
[[nodiscard]] constexpr int CountRightZero(const T value) noexcept {
  // Width from sizeof, not MinMax<T>::digits(): digits() follows
  // std::numeric_limits and drops the sign bit, which is one short of the
  // bit count these builtins actually operate on.
  static_assert(base::is_integral_v<T> && !base::is_signed_v<T>,
                "bit counting is defined for unsigned integers only");
  constexpr int kDigits = static_cast<int>(sizeof(T) * 8);
  if (value == 0)
    return kDigits;
#if defined(_MSC_VER) && !defined(__clang__)
  unsigned long index = 0;
  if constexpr (kDigits <= 32) {
    ::_BitScanForward(&index, static_cast<unsigned long>(value));
  } else {
    ::_BitScanForward64(&index, static_cast<unsigned long long>(value));
  }
  return static_cast<int>(index);
#else
  if constexpr (kDigits <= 32) {
    return __builtin_ctz(static_cast<unsigned int>(value));
  } else {
    return __builtin_ctzll(static_cast<unsigned long long>(value));
  }
#endif
}

}  // namespace base

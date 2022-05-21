// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

unsigned CountLeadingZeros(auto val) {
  if (val == 0)
    return 32;

#if __has_builtin(__builtin_clz) || defined(__GNUC__)
  return __builtin_clz(val);
#elif defined(_MSC_VER)
  unsigned long Index;
  _BitScanReverse(&Index, val);
  return Index ^ 31;
#endif
}

inline unsigned Log2(auto value) {
  if constexpr (ku32Bounds.test(value)) {
    return 31 - CountLeadingZeros(value);
  }
  return 63 - CountLeadingZeros(value);
}

// not a template by desing
constexpr inline bool IsPowerOf2(u32 Value) {
  return Value && !(Value & (Value - 1));
}

constexpr inline bool IsPowerOf2(u64 Value) {
  return Value && !(Value & (Value - 1));
}

// trim to the last power of two, e.g 42 becomes 32
inline u64 LastPowerOf2(u64 x) {
  return x == 1 ? 1 : 1 << (63 - CountLeadingZeros(x));
}
inline u32 LastPowerOf2(u32 x) {
  return x == 1 ? 1 : 1 << (31 - CountLeadingZeros(x));
}

#if 0
// get the next power of two, e.g 432becomes 64
inline auto NextPowerOf2(auto x) {
  return x == 1 ? 1 : 1 << (NBitCount<decltype(x)>() - CountLeadingZeros(x - 1));
}
#endif

inline u32 NextPowerOf2(u32 x) {
  return x == 1 ? 1 : 1 << (32 - CountLeadingZeros(x - 1));
}
inline u64 NextPowerOf2(u64 x) {
  return x == 1 ? 1 : 1 << (64 - CountLeadingZeros(x - 1));
}

// compile time/slow path of above
constexpr inline auto NextPowerOf2_Compile(auto val) {
  val |= (val >> 1);
  val |= (val >> 2);
  val |= (val >> 4);
  val |= (val >> 8);
  val |= (val >> 16);
  // i dont know if you could consider this cursed... probably? :P
  if constexpr (Is64Bit<decltype(val)>()) {
    val |= (val >> 32);
  }
  return val + 1;
}
}  // namespace base
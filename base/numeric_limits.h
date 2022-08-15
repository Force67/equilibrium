// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Utilities for getting the bounds of integer types.
#pragma once

#include <base/arch.h>

namespace base {
template <typename T>
struct MinMax {
  static constexpr T min() { return 0; }
  static constexpr T max() { return 0; }
  static constexpr T digits() { return 0; }
};

template <typename T>
consteval bool TestMinMaxCT(const T v) {
  return v >= MinMax<T>::min() && v <= MinMax<T>::max();
}

template <typename T>
bool TestMinMax(const T v) {
  return v >= MinMax<T>::min() && v <= MinMax<T>::max();
}

template <>
struct MinMax<u8> {
  static constexpr u8 min() { return 0; }
  static constexpr u8 max() { return 255; }
  static constexpr u8 digits() { return 8; }
};

template <>
struct MinMax<i8> {
  static constexpr i8 min() { return -127 - 1; }
  static constexpr i8 max() { return 127; }
  static constexpr i8 digits() { return 7; }
};

template <>
struct MinMax<u16> {
  static constexpr u16 min() { return 0; }
  static constexpr u16 max() { return 65535; }
  static constexpr u16 digits() { return 16; }
};

template <>
struct MinMax<i16> {
  static constexpr i16 min() { return -32767 - 1; }
  static constexpr i16 max() { return 32767; }
  static constexpr i16 digits() { return 15; }
};

template <>
struct MinMax<u32> {
  static constexpr u32 min() { return 0; }
  static constexpr u32 max() { return 0xffffffff; }
  static constexpr u32 digits() { return 32; }
};

template <>
struct MinMax<i32> {
  static constexpr i32 min() { return -2147483647 - 1; }
  static constexpr i32 max() { return 2147483647; }
  static constexpr i32 digits() { return 31; }
};

template <>
struct MinMax<u64> {
  static constexpr u64 min() { return 0; }
  static constexpr u64 max() { return 0xffffffffffffffffu; }
  static constexpr u64 digits() { return 64; }
};

template <>
struct MinMax<i64> {
  static constexpr i64 min() { return -9223372036854775807 - 1; }
  static constexpr i64 max() { return 9223372036854775807; }
  static constexpr i64 digits() { return 63; }
};
}  // namespace base
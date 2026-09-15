// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// STL-free number-to-string conversions, over base::FormatTo -- which
// formats integers itself, so none of this reaches snprintf.
#pragma once

#include <base/arch.h>
#include <base/strings/format.h>
#include <base/strings/xstring.h>

namespace base {
namespace detail {

// One buffer size for every integer: 20 digits for u64, a sign, and a
// terminator, rounded up.
template <typename T>
inline String IntegerToString(T value) {
  char buffer[24];
  const mem_size length = FormatTo(buffer, sizeof(buffer), "{}", value);
  return String(buffer, length);
}

}  // namespace detail

inline String ToString(int value) {
  return detail::IntegerToString(value);
}

inline String ToString(unsigned int value) {
  return detail::IntegerToString(value);
}

inline String ToString(long value) {
  return detail::IntegerToString(value);
}

inline String ToString(unsigned long value) {
  return detail::IntegerToString(value);
}

inline String ToString(long long value) {
  return detail::IntegerToString(value);
}

inline String ToString(unsigned long long value) {
  return detail::IntegerToString(value);
}

// The float overloads keep their historical precision: one decimal for
// float, two for double.
inline String ToString(float value) {
  char buffer[64];
  const mem_size length = FormatTo(buffer, sizeof(buffer), "{:.1f}", value);
  return String(buffer, length);
}

inline String ToString(double value) {
  char buffer[64];
  const mem_size length = FormatTo(buffer, sizeof(buffer), "{:.2f}", value);
  return String(buffer, length);
}

}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Utilities for comparing strings, without <string.h>.
#pragma once

#include <base/arch.h>
#include <base/meta/traits.h>
#include <base/numeric_limits.h>

namespace base {

// Ordering comparison of two null-terminated strings: negative if lhs sorts
// first, zero if equal, positive if rhs sorts first.
//
// The result is int, not mem_size. An unsigned return cannot express "sorts
// first" at all: every negative result wraps to a huge positive one, so
// `Strcmp(a, b) < 0` is false whatever the strings are.
//
// Comparison is by unsigned character value, as the C function is, so it does
// not depend on whether plain char is signed on the target.
template <typename T>
inline constexpr int Strcmp(const T* lhs, const T* rhs) {
  using UChar = make_unsigned_t<T>;
  for (;;) {
    const UChar c1 = static_cast<UChar>(*lhs++);
    const UChar c2 = static_cast<UChar>(*rhs++);
    if (c1 != c2)
      return c1 < c2 ? -1 : 1;
    if (c1 == 0)
      return 0;
  }
}

// As Strcmp, but compares at most |limit| characters. A run of |limit| equal
// characters compares equal even if the strings differ later.
template <typename T>
inline constexpr int Strncmp(const T* lhs, const T* rhs, mem_size limit) {
  using UChar = make_unsigned_t<T>;
  for (mem_size i = 0; i < limit; ++i) {
    const UChar c1 = static_cast<UChar>(lhs[i]);
    const UChar c2 = static_cast<UChar>(rhs[i]);
    if (c1 != c2)
      return c1 < c2 ? -1 : 1;
    if (c1 == 0)
      return 0;
  }
  return 0;
}

// Whether two null-terminated strings are equal. Most callers want this and
// not an ordering, and it cannot be misread the way `Strcmp(a, b)` can.
template <typename T>
inline constexpr bool StrEqual(const T* lhs, const T* rhs) {
  return Strcmp(lhs, rhs) == 0;
}

}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Utilies for comparing strings.
#pragma once

#include <base/numeric_limits.h>

namespace base {

template <typename T>
concept SmallCharOptimizable = requires(T a) {
  {sizeof(T) == sizeof(byte)};
};

template <typename T>
inline mem_size Strcmp(const T* lhs,
                       const T* rhs,
                       mem_size limit = MinMax<mem_size>::max()) requires
    SmallCharOptimizable<T> {
  T c1, c2;

  while (limit-- && (c1 = *lhs++) == (c2 = *rhs++)) {
    if (c1 == 0)
      return 0;
  }

  // We can't just return c1 - c2, because the difference might be greater than
  // INT_MAX.
  return (static_cast<u8>(c1) - static_cast<u8>(c2));
}

inline constexpr mem_size Strcmp(const char16_t* pString1,
                                 const char16_t* pString2) {
  static_assert(sizeof(int) > sizeof(u16), "Expected int to be greater than u16");

  char16_t c1, c2;

  while ((c1 = *pString1++) == (c2 = *pString2++)) {
    if (c1 == 0)  // If we've reached the end of the string with no difference...
      return 0;
  }
  return (static_cast<u16>(c1) - static_cast<u16>(c2));
}

// specialized for char32
inline constexpr mem_size Strcmp(const char32_t* pString1,
                                 const char32_t* pString2) {
  char32_t c1, c2;

  while ((c1 = *pString1++) == (c2 = *pString2++)) {
    if (c1 == 0)  // If we've reached the end of the string with no difference...
      return 0;
  }

  // We can't just return c1 - c2, because the difference might be greater than
  // INT_MAX.
  return (static_cast<u32>(c1) > static_cast<u32>(c2)) ? 1 : -1;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/arch.h>
#include <type_traits>

namespace base {

template <typename T>
inline constexpr mem_size CalculateStringLength(const T* p, mem_size max_size) {
  DCHECK(p);

  const auto* s = p;
  while (*s && max_size--)
    s++;
  return static_cast<mem_size>(s - p);
}

template <typename T>
inline constexpr mem_size FindNullTerminator(const T* p, mem_size n /*max_size*/) {
  DCHECK(p);

  const auto* s = p;
  for (; n > 0; --n, ++s) {
    if (*s == 0)
      return static_cast<mem_size>(s - p);
  }
  return 0;
}

template <typename T>
concept SmallCharOptimizable = requires(T a) {
  {sizeof(T) == sizeof(i8)};
};

template <typename T>
inline constexpr mem_size Strcmp(const char* lhs,
                                 const char* rhs) requires SmallCharOptimizable<T> {
  T c1, c2;

  while ((c1 = *lhs++) == (c2 = *rhs++)) {
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

#if 0
auto Min(auto a, auto b) {
  return (b < a) ? b : a;
}

template <typename TChar>
mem_size FindLastOf(const TChar* haystack,
                    const mem_size haystack_length,
                    const TChar* needle,
                    mem_size pos,
                    mem_size needle_length) {
  if (!haystack || haystack_length <= 0) {
    return kNotFoundPos;
  }

  if (!needle || needle_length <= 0) {
    return kNotFoundPos;
  }

  pos = base::Min(pos, haystack_length - 1);
  const TChar* i = haystack + pos;
  for (;; --i) {
    if (base::Find(haystack, haystack_length, needle, needle_length, *i) !=
        kNotFoundPos) {
      return i - haystack;
    }

    if (i == haystack)
      break;
  }

  return kNotFoundPos;
}
#endif

}  // namespace base
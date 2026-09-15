// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/arch.h>
#include <base/numeric_limits.h>

namespace base {

template <typename T>
inline constexpr mem_size CountStringLength(const T* p,
                                            mem_size limit = MinMax<mem_size>::max()) {
  // DCHECK(p);

  // For unlimited single-byte scans, hand the work to the builtin. It folds a
  // string literal to a constant, which the hand-rolled loop below does not
  // reliably do through an inlining chain -- and a caller that then copies
  // `CountStringLength(literal)` bytes out of that literal looks to the
  // compiler like it may read past the end. It also lowers to the tuned libc
  // routine instead of a byte loop.
  // MSVC has no __builtin_strlen, so there the loop below does the work.
#if !defined(_MSC_VER) || defined(__clang__)
  if constexpr (sizeof(T) == 1) {
    if (!__builtin_is_constant_evaluated() && limit == MinMax<mem_size>::max())
      return __builtin_strlen(reinterpret_cast<const char*>(p));
  }
#endif

  // Indexed rather than a pointer walk with a `limit--` side effect in the
  // condition: the same scan, but one the optimizer can evaluate for a known
  // literal, which is what keeps a caller copying CountStringLength(literal)
  // characters out of it from looking like an over-read.
  mem_size n = 0;
  while (n != limit && p[n])
    ++n;
  return n;
}

// The first occurrence of |needle| in the null-terminated |haystack|, or
// nullptr. Searching for '\0' finds the terminator, as strchr does.
template <typename T>
inline constexpr const T* FindChar(const T* haystack, T needle) {
  for (;; ++haystack) {
    if (*haystack == needle)
      return haystack;
    if (*haystack == 0)
      return nullptr;
  }
}

// The last occurrence of |needle|, or nullptr. As strrchr, searching for
// '\0' finds the terminator.
template <typename T>
inline constexpr const T* FindLastChar(const T* haystack, T needle) {
  const T* found = nullptr;
  for (;; ++haystack) {
    if (*haystack == needle)
      found = haystack;
    if (*haystack == 0)
      return found;
  }
}

template <typename T>
inline constexpr mem_size FindNullTerminator(const T* p, mem_size n /*max_size*/) {
  // DCHECK(p);

  const auto* s = p;
  for (; n > 0; --n, ++s) {
    if (*s == 0)
      return static_cast<mem_size>(s - p);
  }
  return 0;
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
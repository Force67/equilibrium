// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/arch.h>
#include <type_traits>
#include <base/numeric_limits.h>

namespace base {

template <typename T>
inline constexpr mem_size CountStringLength(const T* p,
                                            mem_size limit = MinMax<mem_size>::max()) {
  // DCHECK(p);

  const auto* s = p;
  while (*s && limit--)
    s++;
  return static_cast<mem_size>(s - p);
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
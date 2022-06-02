// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/arch.h>

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
inline constexpr T* Find(const T* p, T c, size_t n) {
  for (; n > 0; --n, ++p) {
    if (*p == c)
      return p;
  }
  return nullptr;
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
}  // namespace base
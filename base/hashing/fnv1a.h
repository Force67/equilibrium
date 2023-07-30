// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

template <typename T, typename TChar = char>
static constexpr T FNV1a(const TChar* str, T hash, T prime) {
  if (!str || !*str)
    return 0;
  while (const int c = *str++) {
    hash ^= c;
    hash *= prime;
  }
  return hash;
}

template <typename TChar = char>
static constexpr u32 FNV1a32(const TChar* str,
                             u32 hash = 0x811c9dc5,
                             u32 prime = 0x1000193) {
  return FNV1a<u32>(str, hash, prime);
}

template <typename TChar = char>
static constexpr u64 FNV1a64(const TChar* str,
                             u64 hash = 0xcbf29ce484222325,
                             u64 prime = 0x100000001b3) {
  return FNV1a<u64>(str, hash, prime);
}
}  // namespace base
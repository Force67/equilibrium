// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
// 64 bit hash implementation of PJW, used in Linux
constexpr u32 ElfHash(const char* str) {
  u32 hash = 0, high;
  while (const char c = *str++) {
    hash = (hash << 4) + c;
    if ((high = hash & 0xF000000000000000))
      hash ^= high >> 24;
    hash &= ~high;
  }
  return hash;
}
}  // namespace base
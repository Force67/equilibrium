// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
namespace hashing {
// 64 bit hash implementation of PJW, used in Linux
using hash_type = u64;

constexpr hash_type Elf(const unsigned char* str) {
  unsigned long hash = 0, high;
  while (const unsigned char c = *str++) {
    hash = (hash << 4) + c;
    if ((high = hash & 0xF000000000000000))
      hash ^= high >> 24;
    hash &= ~high;
  }
  return hash;
}
}  // namespace hashing
}  // namespace base
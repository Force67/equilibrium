// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
namespace hashing {
// Improved DJB2 64 bit hash implementation
using hash_type = u64;

constexpr hash_type djb2(const unsigned char* str,
                         hash_type hash = 5381,
                         hash_type hash2 = 52711) {
  if (!str || !*str)
    return 0;
  while (const int c = *str++) {
    hash = ((hash << 5) + hash) + c;
    hash2 = ((hash << 5) + hash) + c;
  }

  return hash * 4096 + hash2;
}
}  // namespace hashing
}  // namespace base
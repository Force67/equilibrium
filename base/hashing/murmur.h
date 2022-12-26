// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
namespace hashing {
// Murmur hash for 64 bit, suitable for general hash-based lookup
// Not for cryptographic purposes, collisions witnessed
using hash_type = u64;

static hash_type Murmur(const char* str, int len, hash_type seed) {
  const hash_type m = 0xc6a4a7935bd1e995LLU;
  const int r = 47;

  u64 h = seed ^ (len * m);

  const u64* data = reinterpret_cast<const u64*>(str);
  const u64* end = (len >> 3) + data;

  while (data != end) {
    u64 k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  switch (len & 7) {
    case 7: h ^= static_cast<u64>(data[6]) << 48;
    case 6: h ^= static_cast<u64>(data[5]) << 40;
    case 5: h ^= static_cast<u64>(data[4]) << 32;
    case 4: h ^= static_cast<u64>(data[3]) << 24;
    case 3: h ^= static_cast<u64>(data[2]) << 16;
    case 2: h ^= static_cast<u64>(data[1]) << 8;
    case 1: h ^= static_cast<u64>(data[0]);
      h *= m;
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}
}  // namespace hashing
}  // namespace base
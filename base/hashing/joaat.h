// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
inline u32 Joaat(const byte* data, mem_size length) {
  u32 hash = 0;

  for (auto i = 0; i < length; ++i) {
    hash += data[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  // Final mixing steps
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

// el speciale
inline u16 Joaat16(const byte* data, mem_size length) {
  u16 hash = 0;

  for (auto i = 0; i < length; ++i) {
    hash += data[i];
    hash += (hash << 5);
    hash ^= (hash >> 3);
  }
  // Final mixing steps
  hash += (hash << 2);
  hash ^= (hash >> 4);
  hash += (hash << 7);
  return hash;
}
}  // namespace base
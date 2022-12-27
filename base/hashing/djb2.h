// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
// This implements "string-hash-64", also known as a 64 bit version of djb2
// https://github.com/mstdokumaci/string-hash-64
constexpr u64 Djb2Hash64(const char* str, u64 hash = 5381, u64 hash2 = 52711) {
  if (!str || !*str)
    return 0;
  while (const i32 c = *str++) {
    hash = ((hash << 5) + hash) + c;
    hash2 = ((hash << 5) + hash) + c;
  }
  return hash * 4096 + hash2;
}

// regular old djb2
constexpr u32 Djb2Hash32(const char* str, u32 hash = 5381) {
  if (!str || !*str)
    return 0;
  while (const i32 c = *str++) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}
}  // namespace base
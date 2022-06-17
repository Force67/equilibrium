// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
// FNV-1a 64 bit hash
using hash_type = u64;
constexpr hash_type fnv_basis = 14695981039346656037ull;
constexpr hash_type fnv_prime = 1099511628211ull;

constexpr hash_type FNV1a64(const char* str, hash_type hash = fnv_basis) {
  return *str ? FNV1a64(str + 1, (hash ^ hash_type(*str)) * fnv_prime) : hash;
}
}  // namespace base
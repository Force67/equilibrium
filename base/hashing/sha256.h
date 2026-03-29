// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Minimal SHA-256 implementation (FIPS 180-4). No external dependencies.
#pragma once

#include <base/arch.h>

namespace base {

struct Sha256Hash {
  u8 bytes[32];
};

// Hash a memory buffer using SHA-256.
Sha256Hash Sha256(const void* data, mem_size size);

// Hash a null-terminated string using SHA-256.
Sha256Hash Sha256(const char* str);

}  // namespace base

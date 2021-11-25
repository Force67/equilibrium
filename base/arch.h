// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Core type definitions to be portable across architectures.
#pragma once

#include <cstdint>

namespace base {
// These reside in the base namespace to avoid conflict, but you are encouraged
// to use these with using namespace;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = u8;
}  // namespace base

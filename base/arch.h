// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Core type definitions to be portable across architectures.
#pragma once

namespace arch_types {
// These reside in the base namespace to avoid conflict, but you are encouraged
// to use these with using namespace;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8 = signed char;
using i16 = short;
using i32 = int;
using i64 = long long;

// use this instead of u8
using byte = u8;

using mem_size = u64;

template <typename T>
struct MinMax {
  T min, max;
};
constexpr MinMax<u8> ku8Bounds{0, 255};
constexpr MinMax<u16> ku16Bounds{0, 65535};
}  // namespace arch_types

// In case of incompatibility with library custom types.
#ifndef BASE_FORBID_ARCHTYPES
using namespace arch_types;
#endif
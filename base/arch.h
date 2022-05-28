// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Core type definitions to be portable across architectures.
#pragma once
// Warning: This header may _never_ include other source files
namespace arch_types {
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
// signed
using i8 = signed char;
using i16 = short;
using i32 = int;
using i64 = long long;
// use this instead of u8
using byte = u8;

using mem_size = u64;
using pointer_size = u64;
using pointer_diff = i64;

template <typename T>
consteval inline bool Is64Bit() {
  // TODO: only on amd64
  return sizeof(T) == 8;
}

template <typename T>
consteval auto NBitCount() {
  // TODO: only on amd64
  return sizeof(T) * 8;
}
}  // namespace arch_types

// In case of incompatibility with library custom types.
#ifndef BASE_FORBID_ARCHTYPES
using namespace arch_types;
#endif
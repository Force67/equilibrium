// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Core type definitions to be portable across architectures.
#pragma once
// Warning: This header may _never_ include other files
// it is equivalent to the type definition for the fusion language.
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
// floating point
using f32 = float;
using f64 = double;

// use this instead of u8 for data
// we guarantee that this type is always sizeof(byte) = 1, on any platform
using byte = u8;

using mem_size = u64;
using size = u64;
using pointer_size = u64;
using pointer_diff = i64;

constexpr mem_size kCharBit = 8;

template <typename T>
consteval inline bool Is64Bit() {
  return sizeof(T) == kCharBit;
}

template <typename T>
consteval auto NBitCount() {
  return sizeof(T) * kCharBit;
}
}  // namespace arch_types

// In case of incompatibility with library custom types.
#ifndef BASE_FORBID_ARCHTYPES
using namespace arch_types;
#endif
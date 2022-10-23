// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Core definitions for compiler specifics.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>

namespace base {
// this will always byteswap
template <typename T>
consteval T ByteSwap_Always(T i, T j = 0u, mem_size n = 0u) {
  return n == sizeof(T) ? j
             : ByteSwap_Always<T>(i >> kCharBit,
                                      (j << kCharBit) | (i & (T)(u8)(-1)), n + 1);
}

// Swap only to LE
template <typename T>
consteval T ByteSwapToLittleEndian(T i, T j = 0u, mem_size n = 0u) {
  if constexpr (base::kIsBigEndian)
    return i;

  return n == sizeof(T) ? j
             : ByteSwapToLittleEndian<T>(i >> kCharBit,
                                        (j << kCharBit) | (i & (T)(u8)(-1)), n + 1);
}

}  // namespace base
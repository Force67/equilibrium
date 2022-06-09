// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Core definitions for compiler specifics.
#pragma once

#include <base/arch.h>

namespace base {

// TODO: NON AMD64
constexpr mem_size kCharBit = 8;

template <typename T>
consteval T BSwap(T i, T j = 0u, mem_size n = 0u) {
  return n == sizeof(T)
             ? j
             : BSwap<T>(i >> kCharBit, (j << kCharBit) | (i & (T)(u8)(-1)), n + 1);
}

// TODO: Swap only when LE
template <typename T>
consteval T BswapLE(T i, T j = 0u, mem_size n = 0u) {
  return n == sizeof(T)
             ? j
             : BswapLE<T>(i >> kCharBit, (j << kCharBit) | (i & (T)(u8)(-1)), n + 1);
}

}  // namespace base
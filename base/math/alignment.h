// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// based off
// https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/Alignment.h
#pragma once

#include <base/check.h>
#include <base/arch.h>
#include <base/math/math_helpers.h>

namespace base {

// must return to mem_size in case our alignment is > UINT_MAX
inline mem_size Log2Align(const mem_size n) {
  DCHECK(n > 0);
  DCHECK(base::IsPowerOf2(n));

  u8 shift = static_cast<u8>(base::Log2(n));
  DCHECK(shift < 64);  // Broken invariant
  return mem_size(1) << shift;
}

consteval auto ShiftLog2(auto shift) {
  return mem_size(1) << shift;
}

// Align 'n' to a multiple of 'to', e.g Align(69, 1024) -> 1024
template <typename T>
constexpr inline T Align(const T n, const T to) {
  // The following line is equivalent to `(Size + Value - 1) / Value * Value`.
  // The division followed by a multiplication can be thought of as a right
  // shift followed by a left shift which zeros out the extra bits produced in
  // the bump; `~(Value - 1)` is a mask where all those bits being zeroed out
  // are just zero.
  // Most compilers can generate this code but the pattern may be missed when
  // multiple functions gets inlined.
  return ((n + to - 1) & ~(to - 1U));
}
static_assert(Align(69, 1024) == 1024, "Alignment failed");

// align to skew
template <typename T>
constexpr inline T Align(const T n, const T to, T skew) {
  skew %= to;
  return ((n + to - 1 - skew) & ~(to - 1U)) + skew;
}
static_assert(Align(69, 1024, 1) == 1025, "Alignment failed");

template <typename T>
inline pointer_size AlignAddress(const void* address, T align) {
  pointer_size ArithAddr = reinterpret_cast<pointer_size>(address);
  static_assert(static_cast<pointer_size>(ArithAddr + align - 1) >= ArithAddr &&
                "Overflow");
  return base::Align(ArithAddr, align);
}

// checks that SizeInBytes is a multiple of the alignment.
template <typename T>
constexpr inline bool IsAligned(T lhs, const mem_size byte_size) {
  return byte_size % lhs == 0;
}
static_assert(IsAligned(4, 16), "Alignment failed");
}  // namespace base
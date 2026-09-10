// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Checked arithmetic for allocation sizes and element counts.
//
// Containers turn a caller-supplied element count into a byte count. Doing
// that with a plain multiply lets a large count wrap, which produces a small
// allocation that the container then fills with the full element count: a heap
// overflow with no diagnostic. The helpers here either return an exact byte
// count or terminate, because the container APIs that need them (push_back,
// reserve, resize) return void and have no channel to report a refusal.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/numeric_limits.h>

namespace base {

// Byte count for |count| elements of |element_size| bytes, or false when the
// product does not fit in a mem_size. Use this where the caller has an error
// channel; use CheckedAllocationSize where it has none.
constexpr bool TryAllocationSize(mem_size count,
                                 mem_size element_size,
                                 mem_size& bytes_out) {
  if (element_size != 0 && count > MinMax<mem_size>::max() / element_size)
    return false;
  bytes_out = count * element_size;
  return true;
}

// Byte count for |count| elements of |element_size| bytes. Terminates when the
// product overflows.
constexpr mem_size CheckedAllocationSize(mem_size count, mem_size element_size) {
  mem_size bytes = 0;
  BASE_FATAL_CHECK(TryAllocationSize(count, element_size, bytes),
                   "allocation size overflows mem_size");
  return bytes;
}

// Sum of two element counts. Terminates on wraparound, because a wrapped sum
// silently understates how much room a container has to make.
constexpr mem_size CheckedCountSum(mem_size a, mem_size b) {
  BASE_FATAL_CHECK(a <= MinMax<mem_size>::max() - b, "element count sum overflows");
  return a + b;
}

// |count| scaled by |factor|. Terminates on overflow, so a wrapped growth
// target can never shrink a container's capacity.
constexpr mem_size CheckedCountProduct(mem_size count, mem_size factor) {
  return CheckedAllocationSize(count, factor);
}

// The smallest power of two that is >= |value|, with 0 mapping to 1.
// Terminates when no power of two fits, which is what makes the usual
// `while (target < required) target *= 2` loop safe: unchecked, that loop
// wraps to zero and spins forever.
constexpr mem_size CheckedRoundUpToPowerOfTwo(mem_size value) {
  mem_size target = 1;
  while (target < value) {
    BASE_FATAL_CHECK(target <= MinMax<mem_size>::max() / 2,
                     "no power of two can hold the requested count");
    target *= 2;
  }
  return target;
}

}  // namespace base

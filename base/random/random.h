// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/numeric_limits.h>

namespace base {
BASE_EXPORT i32 RandomInt(i32 start = MinMax<i32>::min(),
                          i32 end = MinMax<i32>::max());
BASE_EXPORT u32 RandomUint(u32 start = 0u, u32 end = MinMax<u32>::max());

BASE_EXPORT uint64_t SourceTrueRandomSeed();
}  // namespace base
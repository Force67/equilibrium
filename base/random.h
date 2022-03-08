// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {
BASE_EXPORT i32 RandomInt(i32 start = INT_MIN, i32 end = INT_MAX);
BASE_EXPORT u32 RandomUint(u32 start = 0u, u32 end = UINT32_MAX);

}  // namespace base
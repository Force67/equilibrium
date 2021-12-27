// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace base {
// mersenne random integer
int RandomInt(int start = INT_MIN, int end = INT_MAX);

// mersenne random unsigned integer
uint32_t RandomUint(uint32_t start = 0u, uint32_t end = UINT32_MAX);
}  // namespace base
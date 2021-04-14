// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <random>

namespace network::tksp {

// gets a new random seed using the mersenne
// twister
uint32_t GetRandomSeed(uint32_t reserve = 0u);
}  // namespace network::tksp
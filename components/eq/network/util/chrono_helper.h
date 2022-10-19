#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <chrono>

namespace network::util {

using namespace std::chrono_literals;

using MilliSeconds = std::chrono::milliseconds;

// returns chrono milliseconds
inline MilliSeconds msec() {
  return std::chrono::duration_cast<MilliSeconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}
}
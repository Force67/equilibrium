// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <chrono>
#include "network_context.h"

namespace network {
namespace {
using namespace std::literals::chrono_literals;

constexpr const auto kGlobalTimeout = 6000ms;

// server defaults
constexpr char kDefaultIp[] = "127.0.0.1";
constexpr int kDefaultPort = 4523;
} // namespace

// your friendly one liner
inline auto msec() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}
}
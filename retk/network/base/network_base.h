// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <chrono>
#include "network_context.h"

namespace network {
namespace {
using namespace std::literals::chrono_literals;

constexpr const auto kGlobalTimeout = 6000ms;

constexpr int kClientVersion = 1;
constexpr int kWorkBufSize = 2048;

// server defaults
constexpr char kDefaultIp[] = "127.0.0.1";
constexpr int kDefaultPort = 4523;
constexpr int kDefaultPortRange = 10;
} // namespace

enum class OpCode : uint32_t {
  // system ops
  kQuit = 0xb9d273f2,
  kJoin = 0x781aa8ec,
  kHeartbeat = 0x7440ad27,

  // user ops
  kData = 0xb36fcb5c
};


// your friendly one liner
inline auto msec() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}
}
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/socket.h>
#include <limits>

namespace network {
// Required to be created before using any network functions
struct Context {
  Context();
  ~Context();

  static bool Initialized();
};

constexpr int kTCPBufSize = 2048;

constexpr uint32_t kTimeout = 3000;
constexpr uint16_t kClientVersion = 1;

constexpr char kDefaultServerIp[] = "127.0.0.1";
constexpr int16_t kDefaultServerPort = 4523;

// port + range will be checked to host on
constexpr int32_t kDefaultPortRange = 10;

}  // namespace network
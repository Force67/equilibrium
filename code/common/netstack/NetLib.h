// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace common {

  namespace constants {
  // default values
  constexpr char kServerIp[] = "127.0.0.1";
  constexpr uint16_t kServerPort = 4523;

  // constants
  constexpr uint32_t kTimeout = 3000;
  constexpr uint32_t kNetworkerThreadIdle = 1;
  constexpr uint16_t kClientVersion = 1;
  }

  void InitNetLib();
  void DestroyNetLib();

  struct ScopedNetContext {
  
  explicit ScopedNetContext() {
	InitNetLib();
  }

  ~ScopedNetContext() {
	DestroyNetLib();
  }
  };
}
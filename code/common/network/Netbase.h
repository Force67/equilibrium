// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <limits>
#include <sockpp/socket.h>
#include "protocol/generated/MessageRoot_generated.h"

namespace network {
  namespace pt = protocol;

  // TODO: move this into NetBuffer
  using FbsBuffer = flatbuffers::FlatBufferBuilder;

  // Flatbuffer aliases
  template <typename T>
  using FbsRef = flatbuffers::Offset<T>;
  using FbsStringRef = flatbuffers::Offset<flatbuffers::String>;

  // replacement for socket_initializer, which doesn't check
  // if the socket was initialized before
  struct Context {
	Context();
	~Context();

	static bool Initialized();
  };

  namespace constants {
	constexpr int kTCPBufSize = 2048;

	constexpr uint32_t kTimeout = 3000;
	constexpr uint32_t kNetworkerThreadIdle = 1;
	constexpr uint16_t kClientVersion = 1;

	constexpr char kServerIp[] = "127.0.0.1";
	constexpr int16_t kServerPort = 4523;
	constexpr int32_t kServerDiscoveryPortRange = 10;
  } // namespace constants
} // namespace network
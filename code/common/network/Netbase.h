// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <limits>
#include <sockpp/socket.h>
#include "protocol/generated/Message_generated.h"

namespace network {
  namespace pt = protocol;

  using FbsBuffer = flatbuffers::FlatBufferBuilder;

  template <typename T>
  using FbsRef = flatbuffers::Offset<T>;
  using FbsStringRef = flatbuffers::Offset<flatbuffers::String >;

  using ScopedSocket = sockpp::socket_initializer;

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
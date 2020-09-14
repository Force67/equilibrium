// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This implements the net library base on which NODA operates
#pragma once

#include <enet/enet.h>
#include "flatbuffers/flatbuffers.h"

namespace noda
{
  namespace net
  {
	template <typename T>
	using FbsOffset = flatbuffers::Offset<T>;

	using FbsBuilder = flatbuffers::FlatBufferBuilder;
	using FbsStringRef = FbsOffset<flatbuffers::String>;

	namespace constants
	{
	  // default values
	  constexpr char kServerIp[] = "127.0.0.1";
	  constexpr uint16_t kServerPort = 4523;

	  // constants
	  constexpr uint32_t kTimeout = 3000;
	  constexpr uint32_t kNetworkerThreadIdle = 1;
	  constexpr uint16_t kClientVersion = 1;
	} // namespace constants

	class NetDelegate
	{
	public:
	  virtual ~NetDelegate() = default;

	  // this is triggered when a connection has been
	  // established
	  virtual void OnConnectRequest() {}

	  // When the client gets accepted by the server
	  virtual void OnConnect() = 0;

	  // We have been dropped
	  virtual void OnDisconnect(uint32_t reason) = 0;

	  // Handle a new message
	  virtual void ProcessPacket(uint8_t *data, size_t length) = 0;
	};
  } // namespace net
} // namespace noda
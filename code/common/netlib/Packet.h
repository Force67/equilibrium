// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace netlib {

  // ref counted packet wrapper
  // to reduce allocations
  class Packet {
  public:
	Packet(ENetPacket *packet);
	~Packet();

	// copy ctor
	Packet(const Packet &p);

	Packet &operator=(const Packet &) = delete;
	Packet &operator=(Packet *) = delete;

	const uint8_t *data() const
	{
	  return _packet->data;
	}

	const size_t length() const
	{
	  return _packet->dataLength;
	}

	inline ENetPacket *GetPacket() const
	{
	  return _packet;
	}

  private:
	ENetPacket *_packet;
  };

  static_assert(sizeof(Packet) == sizeof(ENetPacket *));
} // namespace netlib
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Packet.h"

#include <cstdio>

namespace netlib {

  Packet::Packet(ENetPacket *packet) :
      _packet(packet)
  {
	// yes we use the packets internal ref counter
	// to prevent the library from freeing it itself
	_packet->referenceCount++;
  }

  Packet::Packet(const Packet &p) :
      _packet(p._packet)
  {
	_packet->referenceCount++;
  }

  Packet::~Packet()
  {
	_packet->referenceCount--;

	if(_packet->referenceCount == 0) {
	  enet_packet_destroy(_packet);
	}
  }
} // namespace netlib
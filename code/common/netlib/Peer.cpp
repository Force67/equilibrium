// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Peer.h"

namespace netlib {

  connectid_t Peer::Id()
  {
	return static_cast<connectid_t>(
	    reinterpret_cast<uintptr_t>(_peer->data));
  }

  void Peer::SetId(connectid_t id)
  {
	_peer->data = reinterpret_cast<void *>(
	    static_cast<uintptr_t>(id));
  }

  int Peer::GetPing()
  {
	return _peer->roundTripTime;
  }

  void Peer::SendReliable(const uint8_t *data, size_t len)
  {
	auto *packet = enet_packet_create(static_cast<const void *>(data), len, ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(_peer, 1, packet);
	enet_packet_destroy(packet);
  }

  void Peer::SetManaged()
  {
	//_peer->
  }

  void Peer::Kick(int code)
  {
	enet_peer_disconnect(_peer, code);

	// TODO: force disconnect
  }
} // namespace netlib
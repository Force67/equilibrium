// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Peer.h"

namespace netlib {

  uint32_t PeerBase::Id()
  {
	return static_cast<uint32_t>(
	    reinterpret_cast<uintptr_t>(_peer->data));
  }

  void PeerBase::SetId(uint32_t id)
  {
	_peer->data = reinterpret_cast<void *>(
	    static_cast<uintptr_t>(id));
  }

  int PeerBase::GetPing()
  {
	return _peer->roundTripTime;
  }

  void PeerBase::SendReliable(const uint8_t *data, size_t len)
  {
	auto *packet = enet_packet_create(static_cast<const void *>(data), len, ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(_peer, 1, packet);
	enet_packet_destroy(packet);
  }

  void PeerBase::Kick(int code)
  {
	enet_peer_disconnect(_peer, code);

	// TODO: force disconnect
  }
} // namespace netlib
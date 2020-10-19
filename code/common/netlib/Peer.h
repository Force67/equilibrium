// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace netlib {
  using connectid_t = uint32_t;

  class Peer {
  public:
	explicit Peer(ENetPeer *p) :
	    _peer(p) {}

	void SendReliable(const uint8_t *data, size_t len);

	void Kick(int code);

	connectid_t Id();
	int GetPing();

	inline ENetPeer *GetPeer()
	{
	  return _peer;
	}

	void SetId(connectid_t);
	void SetManaged();

  private:
	ENetPeer *_peer;
  };

  static_assert(sizeof(Peer) == sizeof(ENetPeer *));
} // namespace netlib
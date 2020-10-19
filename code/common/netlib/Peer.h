// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace netlib {

  class PeerBase {
  public:
	explicit PeerBase(ENetPeer *p) :
	    _peer(p) {}

	void SendReliable(const uint8_t *data, size_t len);

	void Kick(int code);

	uint32_t Id();
	int GetPing();

	inline ENetPeer *GetPeer()
	{
	  return _peer;
	}

	void SetId(uint32_t);
  private:
	ENetPeer *_peer;
  };

  static_assert(sizeof(PeerBase) == sizeof(ENetPeer *));
} // namespace netlib
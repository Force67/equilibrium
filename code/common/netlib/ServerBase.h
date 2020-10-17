// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace netlib {

  class NetServerBase {
  public:
	explicit NetServerBase(uint16_t port);
	~NetServerBase();

	inline bool Good() const
	{
	  return _host;
	}

	void Listen();

	virtual bool OnConnection(ENetPeer *) = 0;
	virtual bool OnDisconnection(ENetPeer *) = 0;
	virtual void OnConsume(ENetPeer *, const uint8_t *data, const size_t len) = 0;

	void BroadcastReliable(uint8_t *data, size_t len);

	size_t GetPeerCount() const;
  private:
	ENetHost *_host;
	ENetEvent _event{};
  };
} // namespace netlib
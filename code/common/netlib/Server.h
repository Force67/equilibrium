// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace netlib {

  class ServerBase {
  public:
	~ServerBase();

	inline bool Good() const
	{
	  return _host != nullptr;
	}

	bool Host(uint16_t port);

	void Listen();

	virtual bool OnConnection(ENetPeer *) = 0;
	virtual bool OnDisconnection(ENetPeer *) = 0;
	virtual void OnConsume(ENetPeer *, const uint8_t *data, const size_t len) = 0;

	// warning: this function is thread unsafe... use as is
	void BroadcastReliable(const uint8_t *data, size_t len, ENetPeer *ex = nullptr);

	size_t GetPeerCount() const;

  private:
	ENetHost *_host = nullptr;
	ENetEvent _event{};
  };
} // namespace netlib
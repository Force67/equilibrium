// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Peer.h"

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

	virtual void OnConnection(PeerBase *){};
	virtual void OnDisconnection(PeerBase *) = 0;
	virtual void OnConsume(PeerBase *, const uint8_t *data, const size_t len) = 0;

	// warning: this function is thread unsafe... use as is
	void BroadcastReliable(const uint8_t *data, size_t len, PeerBase *ex = nullptr);

	bool SendReliable(connectid_t, const uint8_t *data, size_t len);

	size_t GetPeerCount() const;

  private:
	ENetPeer *PeerById(connectid_t);

	ENetHost *_host = nullptr;
	ENetEvent _event{};
  };
} // namespace netlib
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Peer.h"
#include "Packet.h"
#include "NetLib.h"

namespace netlib {

  class NetServer {
  public:
	~NetServer();

	inline bool Good() const
	{
	  return _host != nullptr;
	}

	bool Host(uint16_t port);

	void Listen();

	virtual void OnConnection(Peer *){};
	virtual void OnDisconnection(Peer *) = 0;
	virtual void OnConsume(Peer *, Packet *) = 0;

	void BroadcastReliable(Packet *, Peer *ex = nullptr);

	bool SendReliableUnsafe(connectid_t id,
	                        const uint8_t *data,
	                        size_t len,
	                        void *userp);

	size_t GetPeerCount() const;

	static void SetDeleter(packetdeleter_t);

  private:
	ENetPeer *PeerById(connectid_t);

	ENetHost *_host = nullptr;
	ENetEvent _event{};
  };
} // namespace netlib
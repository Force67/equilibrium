// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Packet.h"

namespace netlib {

  class Client {
  public:
	Client();
	~Client();

	inline bool Good() const
	{
	  return _host;
	}

	virtual void OnConnection() = 0;
	virtual void OnDisconnected(int) = 0;
	virtual void OnConsume(Packet *) = 0;

	bool SendReliable(uint8_t *ptr, size_t size);

	void Disconnect();
	bool Connect(const char *address, uint16_t port);

	void Tick();

  private:
	ENetHost *_host = nullptr;
	ENetPeer *_serverPeer = nullptr;
	ENetAddress _address{};
	ENetEvent _event{};

	size_t _dataRecieved = 0;
  };
} // namespace netlib
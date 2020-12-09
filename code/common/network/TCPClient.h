// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include <sockpp/tcp_connector.h>
#include <utility/DetachedQueue.h>

namespace network {

  class NetworkedClientComponent {
  public:
	virtual ~NetworkedClientComponent() = default;

	// called when connection to dest succeeds
	virtual void OnConnection(const sockpp::inet_address &){};

	// We have been dropped
	virtual void OnDisconnected(int reason) = 0;

	// Handle a new message
	virtual void ConsumeMessage(const uint8_t *ptr, size_t len) = 0;
  };

  struct OutPacket {
	FbsBuffer buffer;
	utility::detached_queue_key<OutPacket> key;
  };

  class TCPClient {
  public:
	bool Update();

	// the connect call is blocking.
	bool Connect(const char *addr, int port);
	void Disconnect();

	// thread safe
	void SendPacket(pt::MsgType, FbsBuffer &buf, FbsRef<void> ref);

	void RegisterComponent(NetworkedClientComponent *);

	std::string LastError() const;

	bool Connected() const
	{
	  return _conn.is_connected();
	}

	const auto GetAddress() const
	{
	  return _addr;
	}

  private:
	sockpp::inet_address _addr;
	sockpp::tcp_connector _conn;

	uint8_t buf[constants::kTCPBufSize]{};
	utility::detached_mpsc_queue<OutPacket> _outQueue;

	std::vector<NetworkedClientComponent *> _listeners;
  };
} // namespace network
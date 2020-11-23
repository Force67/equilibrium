// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include <sockpp/tcp_connector.h>
#include <utility/DetachedQueue.h>

namespace network {

  class TCPClientConsumer {
  public:
	virtual ~TCPClientConsumer() = default;

	// We have been dropped
	virtual void OnDisconnect(int reason) = 0;

	// Handle a new message
	virtual void ConsumeMessage(const uint8_t *ptr, size_t len) = 0;
  };

  struct OutPacket {
	FbsBuffer buffer;
	utility::detached_queue_key<OutPacket> key;
  };

  class TCPClient {
  public:
	TCPClient(TCPClientConsumer &);

	// the connect call is blocking.
	bool Connect(const char *addr, int16_t port);
	void Disconnect();

	void Update();

	// thread safe
	void SendPacket(pt::MsgType, FbsBuffer &buf, FbsRef<void> ref);

	std::string LastError() const;

	bool Connected() const {
		return _conn.is_connected();
	}

	const auto GetAddress() const {
		return _addr;
	}

  private:
	sockpp::inet_address _addr;
	sockpp::tcp_connector _conn;

	uint8_t buf[constants::kTCPBufSize]{};
	utility::detached_mpsc_queue<OutPacket> _outQueue;

	TCPClientConsumer &_consumer;
  };
} // namespace network
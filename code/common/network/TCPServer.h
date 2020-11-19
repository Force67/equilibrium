// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include <sockpp/tcp_acceptor.h>
#include "flatbuffers/flatbuffers.h"

namespace network {

  class TCPServerConsumer {
  public:
	virtual ~TCPServerConsumer() = default;

	virtual void ConsumeMessage() = 0;
  };

  class TCPServer {
  public:
	explicit TCPServer(TCPServerConsumer &consumer);

	bool Host(int16_t port);

	void Tick();

  private:
	sockpp::tcp_acceptor _acc;
	std::vector<sockpp::tcp_socket> _sockets;

	TCPServerConsumer &_consumer;
  };
} // namespace network
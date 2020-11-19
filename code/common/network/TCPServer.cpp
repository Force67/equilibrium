// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "TCPServer.h"

namespace network {

  TCPServer::TCPServer(TCPServerConsumer &consumer) :
      _consumer(consumer)
  {
	_acc.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);
  }

  bool TCPServer::Host(int16_t port)
  {
	return _acc.open(port);
  }

  void TCPServer::Tick()
  {
	sockpp::tcp_socket sock = _acc.accept();
  }
} // namespace network
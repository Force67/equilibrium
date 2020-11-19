// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include <sockpp/tcp_acceptor.h>

namespace network {

  using connectionid_t = uint32_t;

  struct TCPPeer {
	explicit TCPPeer(sockpp::tcp_socket &s)
	{
	  sock = std::move(s);
	}

	inline bool open() const
	{
	  return sock.is_open();
	}

	sockpp::inet_address addr;
	connectionid_t id;
	sockpp::tcp_socket sock;
  };
} // namespace network
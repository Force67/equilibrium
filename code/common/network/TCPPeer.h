// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include <sockpp/tcp_acceptor.h>

#undef max

namespace network {

  using connectid_t = uint32_t;
  constexpr uint32_t invalid_connectid = std::numeric_limits<uint32_t>::max();

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
	connectid_t id;
	sockpp::tcp_socket sock;
  };
} // namespace network
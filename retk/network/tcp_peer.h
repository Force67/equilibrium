// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_acceptor.h>
#include "context.h"

#if defined(max)
#undef max
#endif

namespace network {

using connectid_t = uint32_t;

// invalid connection
constexpr uint32_t kInvalidConnectId = std::numeric_limits<uint32_t>::max();

// address all peers
constexpr uint32_t kAllConnectId = kInvalidConnectId - 1;

struct TCPPeer {
  explicit TCPPeer(sockpp::tcp_socket& s) { sock = std::move(s); }

  inline bool open() const { return sock.is_open(); }

  sockpp::inet_address addr;
  connectid_t id;
  sockpp::tcp_socket sock;
};
}  // namespace network
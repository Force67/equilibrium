// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_acceptor.h>
#include <network/base/network_base.h>

#if defined(max)
#undef max
#endif

namespace network {

using connectid_t = uint32_t;

// invalid connection
constexpr uint32_t kInvalidConnectId = std::numeric_limits<uint32_t>::max();

// address all peers
constexpr uint32_t kAllConnectId = kInvalidConnectId - 1;

class TCPPeer {
  friend class TCPServer;

 public:
  explicit TCPPeer(sockpp::tcp_socket&, connectid_t, sockpp::inet_address);

  inline bool Open() const { return sock_.is_open(); }

  void Immortalize() { mayDie_ = false; }
  void Kill();

  // extend the lifetime of the peer by kDeathTimeout milliseconds
  void Touch();
  // we are due to be killed
  bool HasDied();

  void Send(const uint8_t *ptr, size_t len);

  const connectid_t Id() const { return id_; }
  const auto Address() const { return addr_; }

 private:
  sockpp::inet_address addr_;
  sockpp::tcp_socket sock_;
  connectid_t id_;
  bool mayDie_ = true;
  std::chrono::milliseconds timestamp_;
};
}  // namespace network
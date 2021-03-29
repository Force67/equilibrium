// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <chrono>
#include <sockpp/tcp_connector.h>
#include <network/base/network_encoding.h>

namespace network {

// invalid connection
constexpr uint32_t kInvalidConnectId = UINT_MAX;

// address all peers
constexpr uint32_t kAllConnectId = kInvalidConnectId - 1;

// your friendly one liner
inline auto msec() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}

class NetworkPeer {
 public:
  explicit NetworkPeer(sockpp::stream_socket, connectid_t, sockpp::inet_address);

  void Immortalize() { immortal_ = true; }
  void Kill();

  void Touch();
  bool HasDied();

  inline bool Open() const { 
	  return sock_.is_open(); 
  }

  void Close() { 
	  sock_.close();
  }

  void Send(const uint8_t* ptr, size_t len);
  size_t Receive(uint8_t* buf, size_t buflen);

  const connectid_t ConnectId() const { return connectId_; }
  const auto Address() const { return address_; }
  const auto GetPing() const { return lastSeen_; }

 private:
  sockpp::stream_socket sock_;
  sockpp::inet_address address_;
  connectid_t connectId_;
  bool immortal_ = false;
  std::chrono::milliseconds lastSeen_{};
};
}
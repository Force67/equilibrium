// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_peer.h"

namespace network {

// keep in mind, by default the timeouts are very generous
// case the touch event is only fired on message receive
constexpr auto const kClientDeadTimeout = 60s;

TCPPeer::TCPPeer(sockpp::tcp_socket& s, connectid_t id, sockpp::inet_address addr) : 
    sock_(std::move(s)), id_(id), addr_(addr) {
}

void TCPPeer::Touch() {
  timestamp_ = msec();
}

bool TCPPeer::HasDied() {
  return mayDie_ && (msec() - timestamp_) > kClientDeadTimeout;
}

void TCPPeer::Kill() {
  if (mayDie_)
      timestamp_ = kClientDeadTimeout + 1s;
}

void TCPPeer::Send(const uint8_t* ptr, size_t len) {
  sock_.write_n(static_cast<const void*>(ptr), len);
}

}
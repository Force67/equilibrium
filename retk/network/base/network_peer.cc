// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "network_peer.h"

namespace network {

using namespace std::chrono_literals;

constexpr auto const kClientDeadTimeout = 2s;

NetworkPeer::NetworkPeer(sockpp::stream_socket sock, connectid_t cid, sockpp::inet_address addr) : 
	sock_(std::move(sock)), connectId_(cid), address_(std::move(addr)) {}

void NetworkPeer::Touch() {
  lastSeen_ = msec();
}

bool NetworkPeer::HasDied() {
  return !immortal_ && (msec() - lastSeen_) > kClientDeadTimeout;
}

void NetworkPeer::Kill() {
  if (!immortal_)
    lastSeen_ = kClientDeadTimeout + 1s;
}

void NetworkPeer::Send(const uint8_t* ptr, size_t len) {
  sock_.write_n(static_cast<const void*>(ptr), len);
}

size_t NetworkPeer::Receive(uint8_t* buf, size_t buflen) {
  return sock_.read_n(static_cast<void*>(buf), buflen);
}

}
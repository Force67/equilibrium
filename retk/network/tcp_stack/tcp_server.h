// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <base/object_pool.h>
#include <network/core/network_base.h>
#include "tcp_peer.h"

namespace network {

class TCPServerDelegate {
 public:
  virtual ~TCPServerDelegate() = default;

  virtual void OnConnection(connectid_t){};
  virtual void OnDisconnection(connectid_t) = 0;
  virtual void ProcessData(connectid_t cid, const uint8_t* data, size_t len) = 0;
};

class TCPServer {
 public:
  explicit TCPServer(TCPServerDelegate&);

  // returns the actual host within the port range
  int16_t Host(int16_t port);

  bool DropPeer(connectid_t);
  TCPPeer* PeerById(connectid_t);

  // not thread safe
  void BroadcastPacket(const uint8_t* data,
                       size_t size,
                       connectid_t excluder = kInvalidConnectId);

  int16_t Port() const { return _port; }
 protected:
  void Tick();

 protected:
  sockpp::tcp_acceptor _acc;
  sockpp::inet_address _addr;
  std::vector<TCPPeer> _peers;

 private:
  TCPServerDelegate& delegate_;
  uint8_t workbuf_[kWorkBufSize]{};

  int16_t _port = 0;
  uint32_t _seed;
};
}  // namespace network
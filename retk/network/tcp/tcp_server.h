// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <base/object_pool.h>
#include <sockpp/tcp_acceptor.h>

#include <network/base/network_host.h>
#include <network/base/network_peer.h>

namespace network {

class TCPServer : public NetworkHost {
 public:
  explicit TCPServer(ServerDelegate&);

  // returns the actual host within the port range
  int16_t TryHost(int16_t port);
  void Update();

  bool DropPeer(connectid_t);
  NetworkPeer* PeerById(connectid_t);

  // not thread safe
  void BroadcastPacket(const uint8_t* data,
                       size_t size,
                       connectid_t excluder = kInvalidConnectId);

  int16_t Port() const { return port_; }

  template <typename T>
  void QueueOutgoingCommand(CommandId id, connectid_t cid, const T& val) {
    QueueCommand(id, cid, reinterpret_cast<const uint8_t*>(&val), sizeof(val));
  }

  template <typename T>
  void QueueBroadcast(CommandId id, const T& val) {
    QueueOutgoingCommand(id, kAllConnectId, val);
  }

  struct Entry;
 protected:

  void QueueCommand(CommandId, connectid_t, const uint8_t* ptr, size_t len);
 protected:
  sockpp::tcp_acceptor acceptor_;
  std::vector<std::unique_ptr<NetworkPeer>> peers_;

 private:
  ServerDelegate& delegate_;
  base::detached_mpsc_queue<Entry> outgoingQueue_;

  int16_t port_ = 0;
  uint32_t seed_;
};
}  // namespace network
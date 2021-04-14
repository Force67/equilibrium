// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <base/object_pool.h>
#include <sockpp/tcp_acceptor.h>

#include <network/tksp/tksp_host.h>

namespace network::tksp {

class Server : public tksp::Host {
 public:
  explicit Server(TkspDelegate& delegate) : tksp::Host(delegate) {}

  // attempt to host (between) port + kPortRange
  bool Host(uint16_t port);

  void Quit();

  // port that we are being hosted on
  uint16_t Port() { return my_address_.port();}

  bool Process();

  // configure the socket stream
  virtual bool SetSocketOptions();
 private:
  PeerBase::Adress my_address_{};
  PeerBase::Adress next_address_{};
  sockpp::tcp_acceptor socket_;

  void Tick();

  // send quit msg!
  bool DropPeer(PeerId);

  // immediately kill the peer
  bool DropPeerNow(PeerId);

  NetworkPeer* PeerById(PeerId);

  // not thread safe
  void BroadcastPacket(const uint8_t* data,
                       size_t size,
                       PeerId excluder = kInvalidConnectId);

  template <typename T>
  void QueueOutgoingCommand(CommandId id, PeerId cid, const T& val) {
    QueueCommand(id, cid, reinterpret_cast<const uint8_t*>(&val), sizeof(val));
  }

  template <typename T>
  void QueueBroadcast(CommandId id, const T& val) {
    QueueOutgoingCommand(id, kAllConnectId, val);
  }

  struct Entry;
 protected:

  void QueueCommand(CommandId, PeerId, const uint8_t* ptr, size_t len);
 protected:

 private:

};
}  // namespace network
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "netbuffer.h"
#include "tcp_peer.h"

#include <utility/detached_queue.h>
#include <utility/object_pool.h>

namespace network {

class TCPServerConsumer {
 public:
  virtual ~TCPServerConsumer() = default;

  virtual void OnConnection(connectid_t){};
  virtual void OnDisconnection(connectid_t) = 0;

  virtual void ConsumeMessage(connectid_t, const uint8_t*, size_t size) = 0;
};

class TCPServer {
 public:
  explicit TCPServer(TCPServerConsumer& consumer);

  // returns the actual host within the port range
  int16_t Host(int16_t port);

  void Tick();

  bool DropPeer(connectid_t);
  TCPPeer* PeerById(connectid_t);

  void SendPacket(connectid_t cid,
                  protocol::MsgType type,
                  FbsBuffer& buffer,
                  FbsRef<void> packet);

  // not threadsafe
  void BroadcastPacket(const uint8_t* data,
                       size_t size,
                       connectid_t excluder = kInvalidConnectId);

  int16_t Port() const { return _port; }

 private:
  uint32_t GetMartinsRandomSeed();

  sockpp::tcp_acceptor _acc;
  sockpp::inet_address _addr;

  std::vector<TCPPeer> _peers;
  TCPServerConsumer& _consumer;

  uint8_t _workbuf[1024]{};

  int16_t _port = 0;
  uint32_t _seed;

  struct Packet {
    FbsBuffer buffer;
    connectid_t cid;
    utility::detached_queue_key<Packet> key;
  };

  utility::object_pool<Packet> _packetPool;
  utility::detached_mpsc_queue<Packet> _queue;
};
}  // namespace network
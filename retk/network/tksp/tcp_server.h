// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/container/detached_queue.h>
#include <base/container/object_pool.h>
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
  uint16_t Port() { return my_address_.port(); }

  bool Process();

  // configure the socket stream
  virtual bool SetSocketOptions() override;

 private:
  template <typename T>
  void QueueOutgoingCommand(Chunkheader::Type type,
                            PeerBase::Id peer_id,
                            const T& val) {
    Host::QueueOutgoingCommand(
        type, peer_id, reinterpret_cast<const uint8_t*>(&val), sizeof(val));
  }

  template <typename T>
  void QueueBroadcast(Chunkheader::Type type, const T& val) {
    QueueOutgoingCommand(type, PeerBase::kAllConnectId, val);
  }

 private:
  PeerBase::Adress my_address_{};
  PeerBase::Adress next_address_{};
  sockpp::tcp_acceptor socket_;
};
}  // namespace network::tksp
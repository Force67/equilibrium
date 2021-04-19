// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_connector.h>
#include <base/container/detached_queue.h>
#include <network/tksp/tksp_host.h>

namespace network::tksp {

class Client : public tksp::Host {
 public:
  explicit Client(TkspDelegate& delegate) : tksp::Host(delegate){};

  bool Connect(const PeerBase::Adress& remote_address);
  void Disconnect();
  bool Connected() const;
  // tick frame
  bool Process();

  // configure the socket stream
  virtual bool SetSocketOptions() override;

 private:
  PeerBase& MainPeer();

  template <typename T>
  void QueueOutgoingCommand(Chunkheader::Type type, const T& val) {
    Host::QueueOutgoingCommand(type,
        MainPeer().id, reinterpret_cast<const uint8_t*>(&val), sizeof(val));
  }

 private:
  sockpp::tcp_connector socket_;
  util::MilliSeconds last_ping_;
};
}  // namespace network::tksp
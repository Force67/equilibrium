// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_connector.h>
#include <base/detached_queue.h>

#include <network/base/network_host.h>

namespace network {

class TCPClient : public NetworkHost {
 public:
  explicit TCPClient(ClientDelegate&);

  // the connect call is blocking.
  bool Connect(const char* addr, int port);
  void Disconnect();
  bool Update();

  bool Connected() const { return connection_.is_connected(); }

  template<typename T>
  void QueueOutgoingCommand(CommandId id, const T& val) {
    QueueCommand(id, reinterpret_cast<const uint8_t*>(&val), sizeof(val));
  }

  struct Entry;
 protected:

  void QueueCommand(CommandId op, const uint8_t* ptr, size_t length);
 protected:
  sockpp::tcp_connector connection_;

 private:
  ClientDelegate& delegate_;
  std::chrono::milliseconds lastPinged_{};
  base::detached_mpsc_queue<Entry> outgoingQueue_;
};
}  // namespace network
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_connector.h>
#include <base/detached_queue.h>
#include <network/core/network_base.h>

namespace network {

class TCPClientDelegate {
 public:
  virtual ~TCPClientDelegate() = default;

  // called when connection to dest succeeds
  virtual void OnConnection(const sockpp::inet_address&){};
  virtual void OnDisconnected(int reason) = 0;
  virtual void ProcessData(const uint8_t* ptr, size_t len) = 0;
};

class TCPClient {
 public:
  explicit TCPClient(TCPClientDelegate&);

  // the connect call is blocking.
  bool Connect(const char* addr, int port);
  void Disconnect();

  std::string LastError() const;

  bool Connected() const { return connection_.is_connected(); }
  const auto GetAddress() const { return address_; }

  void Send(OpCode op, const uint8_t* ptr, size_t len);

  struct Entry;
 protected:
  // you are supposed to wrap this stuff
  bool Update();

 protected:
  sockpp::inet_address address_;
  sockpp::tcp_connector connection_;

 private:
  uint8_t workbuf_[kWorkBufSize]{};
  TCPClientDelegate& delegate_;
  std::chrono::milliseconds timestamp_{};
  base::detached_mpsc_queue<Entry> queue_;
};
}  // namespace network
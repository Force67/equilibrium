// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_client.h"
#include <base/object_pool.h>

using namespace std::chrono_literals;

namespace network {

TCPClient::TCPClient(TCPClientDelegate& delegate) : delegate_(delegate) {}

bool TCPClient::Connect(const char* addr, int port) {
  if (!addr)
    addr = "localhost";

  address_ = sockpp::inet_address(addr, static_cast<in_port_t>(port));

  bool result;
  result = connection_.connect(address_);
  result = connection_.set_non_blocking(true);

  // no timeouts
  connection_.read_timeout(0ms);
  connection_.write_timeout(0ms);

  /*int32_t val = 0;
  _conn.get_option(IPPROTO_TCP, TCP_KEEPCNT, &val);*/

  result = connection_.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);

  if (!result && connection_.is_connected()) {
    connection_.reset();
    return false;
  }

  delegate_.OnConnection(address_);
  return result;
}

std::string TCPClient::LastError() const {
  return connection_.last_error_str();
}

void TCPClient::Disconnect() {
    // TODO: host the server for a bit
  // Update();

  delegate_.OnDisconnected(1);

  // Disconnect by force
  connection_.reset();
}

bool TCPClient::Update() {
  if (!connection_.is_connected()) {
    delegate_.OnDisconnected(1337);
    return false;
  }

  ssize_t n = 0;
  while ((n = connection_.read(workbuf_, sizeof(workbuf_))) > 0) {
    delegate_.ProcessData(workbuf_, n);
  }

  return true;
}
}  // namespace network
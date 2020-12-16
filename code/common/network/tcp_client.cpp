// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_client.h"
#include <utility/object_pool.h>

using namespace std::chrono_literals;

namespace network {
inline utility::object_pool<OutPacket> s_packetPool;

bool TCPClient::Connect(const char* addr, int port) {
  if (!addr)
    addr = "localhost";

  _addr = sockpp::inet_address(addr, static_cast<in_port_t>(port));

  bool result;
  result = _conn.connect(_addr);
  result = _conn.set_non_blocking(true);

  // no timeouts
  _conn.read_timeout(0ms);
  _conn.write_timeout(0ms);

  /*int32_t val = 0;
  _conn.get_option(IPPROTO_TCP, TCP_KEEPCNT, &val);*/

  result = _conn.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);

  if (!result && _conn.is_connected()) {
    _conn.reset();
    return false;
  }

  for (NetworkedClientComponent* delegate : _listeners)
    delegate->OnConnection(_addr);

  return result;
}

void TCPClient::RegisterComponent(NetworkedClientComponent* listener) {
  _listeners.push_back(listener);
}

std::string TCPClient::LastError() const {
  return _conn.last_error_str();
}

void TCPClient::Disconnect() {
  // Update();

  // Disconnect by force
  _conn.reset();
}

bool TCPClient::SendPacket(pt::MsgType type, FbsBuffer& buf, FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);

  OutPacket* item = s_packetPool.construct();
  item->buffer = std::move(buf);

  _outQueue.push(&item->key);

  return true;
}

bool TCPClient::Update() {
  if (!_conn.is_connected())
    return false;

  ssize_t n = 0;
  while ((n = _conn.read(buf, sizeof(buf))) > 0) {
    for (auto* it : _listeners)
      it->ConsumeMessage(buf, n);
  }

  while (auto* packet = _outQueue.pop(&OutPacket::key)) {
    _conn.write_n(packet->buffer.GetBufferPointer(), packet->buffer.GetSize());

    s_packetPool.destruct(packet);
  }

  return true;
}
}  // namespace network
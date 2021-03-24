// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_client.h"

#include <network/core/network_packet.h>
#include <base/object_pool.h>

using namespace std::chrono_literals;

namespace network {

struct TCPClient::Entry {
  //Packet packet;
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Entry> key;
};
static base::object_pool<TCPClient::Entry> s_Pool;

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


  Send(OpCode::kQuit, )

    // TODO: host the server for a bit
  // Update();

  //connection_.reset();
}

void TCPClient::Send(OpCode op, const uint8_t* ptr, size_t len) {
  len += sizeof(PacketHeader);
  auto data = std::make_unique<uint8_t[]>(len);

  auto *header = reinterpret_cast<PacketHeader*>(data[0]);
  header->op = op;
  header->flags = 0;
  header->crc = 0;

  std::memcpy(data.get() + sizeof(PacketHeader), ptr, len);

  Entry* item = s_Pool.construct();
  item->dataSize = static_cast<uint32_t>(len);
  item->data = std::move(data);
  queue_.push(&item->key);
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

  while (auto* entry = queue_.pop(&Entry::key)) {
    uint8_t* data = entry->data.get();
    // shitcode...
    bool hasQuit = reinterpret_cast<PacketHeader*>(data)->op == OpCode::kQuit;
    connection_.write_n(data, entry->dataSize);

    s_Pool.destruct(entry);

    if (hasQuit) {
      delegate_.OnDisconnected(1);
      connection_.close();
    }
  }

  return true;
}
}  // namespace network
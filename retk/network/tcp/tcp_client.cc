// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_client.h"
#include <base/object_pool.h>
#include <network/util/sock_util.h>
#include <network/base/network_encoding.h>

using namespace std::chrono_literals;

namespace network {

constexpr int kTCPKeepAliveSeconds = 45;
constexpr auto kTCPClientPingRate = 500ms;

struct TCPClient::Entry {
  // Packet packet;
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Entry> key;
};
static base::object_pool<TCPClient::Entry> s_Pool;

TCPClient::TCPClient(ClientDelegate& delegate)
    : delegate_(delegate), NetworkHost(connection_) {}

bool TCPClient::Connect(const char* addr, int port) {
  if (!addr)
    addr = "localhost";

  address_ = sockpp::inet_address(addr, static_cast<in_port_t>(port));

  bool result;
  result = connection_.connect(address_);
  result = connection_.set_non_blocking(true);
  connection_.read_timeout(0ms);
  connection_.write_timeout(0ms);

  result = util::SetTCPKeepAlive(connection_, true, kTCPKeepAliveSeconds);

  if (!result && connection_.is_connected()) {
    connection_.reset();
    return false;
  }

  JoinCommand command{kEncodingVersion};
  QueueOutgoingCommand(CommandId::kIQuit, command);

  delegate_.OnConnection(address_);
  return result;
}

void TCPClient::Disconnect() {
  // quit processing on the next frame, after we made sure the
  // server got the message
  QuitCommand command{QuitReason::kIWantToQuit};
  QueueOutgoingCommand(CommandId::kIQuit, command);
}

void TCPClient::QueueCommand(CommandId commandId,
                             const uint8_t* ptr,
                             size_t length) {
  // allocate everything in one go for speed reasons.
  length += sizeof(Chunkheader);
  auto data = std::make_unique<uint8_t[]>(length);

  auto* header = reinterpret_cast<Chunkheader*>(data[0]);
  header->id = commandId;
  header->crc = 0;

  if (ptr && length > 0)
    std::memcpy(data.get() + sizeof(Chunkheader), ptr, length);

  Entry* item = s_Pool.construct();
  item->dataSize = static_cast<uint32_t>(length);
  item->data = std::move(data);
  outgoingQueue_.push(&item->key);
}

bool TCPClient::Update() {

    // we got booted by the server
  if (!connection_.is_connected()) {
    delegate_.OnDisconnected(QuitReason::kIGotKicked);
    return false;
  }

  // todo: fragmented headers...
  ssize_t n = 0;
  while ((n = connection_.read(chunkbuf_, sizeof(chunkbuf_))) > 0) {
    delegate_.ProcessData(chunkbuf_, n);
  }

  while (auto* entry = outgoingQueue_.pop(&Entry::key)) {
    uint8_t* data = entry->data.get();
    // shitcode...
    bool hasQuit = reinterpret_cast<Chunkheader*>(data)->id == CommandId::kIQuit;
    connection_.write_n(data, entry->dataSize);

    s_Pool.destruct(entry);

    if (hasQuit) {
      delegate_.OnDisconnected(QuitReason::kIWantToQuit);
      connection_.close();
    }
  }

  // let the server know that we are still alive
  if (timer_ >= kTCPClientPingRate) {
    QueueCommand(CommandId::kIPing, nullptr, 0);
  }

  return true;
}
}  // namespace network
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_server.h"
#include <network/util/sock_util.h>
#include <ctime>

namespace network {

constexpr int kKeepAliveSeconds = 45;
constexpr int kDefaultPortRange = 10;

struct TCPServer::Entry {
  connectid_t cid;
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Entry> key;
};

static base::object_pool<TCPServer::Entry> s_Pool;

TCPServer::TCPServer(ServerDelegate& delegate)
    : delegate_(delegate), NetworkHost(acceptor_), seed_(GetSeed()) {}

int16_t TCPServer::TryHost(int16_t port) {
  port_ = -1;

  for (int32_t i = 0; i < kDefaultPortRange; i++) {
    if (acceptor_.open(port)) {
      port_ = port;
      break;
    }

    port++;
  }

  if (port_ != -1) {
    acceptor_.set_non_blocking();
    util::SetTCPKeepAlive(acceptor_, true, kKeepAliveSeconds);
  }

  return port_;
}

bool TCPServer::DropPeer(connectid_t cid) {
  if (TCPPeer* peer = PeerById(cid)) {
    // trigger the disconnection event on the next frame
    peer->Kill();
    // free the socket
    peer->sock_.close();
    return true;
  }

  return false;
}

void TCPServer::BroadcastPacket(const uint8_t* data,
                                size_t size,
                                connectid_t excluder /* = invalidid_t */) {
  for (auto& it : peers_) {
    if (it.Id() == excluder)
      continue;

    it.sock_.write_n(static_cast<const void*>(data), size);
  }
}

TCPPeer* TCPServer::PeerById(connectid_t id) {
  auto it = std::find_if(peers_.begin(), peers_.end(),
                         [&](TCPPeer& it) { return it.Id() == id; });

  if (it == peers_.end())
    return nullptr;

  return &(*it);
}

void TCPServer::QueueCommand(CommandId commandId,
                             connectid_t connectId,
                             const uint8_t* ptr,
                             size_t len) {
  len += sizeof(Chunkheader);
  auto data = std::make_unique<uint8_t[]>(len);

  auto* header = reinterpret_cast<Chunkheader*>(data[0]);
  header->id = commandId;
  header->crc = 0;

  std::memcpy(data.get() + sizeof(Chunkheader), ptr, len);

  Entry* item = s_Pool.construct();
  item->cid = connectId;
  item->dataSize = static_cast<uint32_t>(len);
  item->data = std::move(data);
  outgoingQueue_.push(&item->key);
}

void TCPServer::Update() {
  {
    // listen for incoming connections
    sockpp::tcp_socket sock = acceptor_.accept(&address_);
    if (sock.is_open()) {
      auto myId = ++seed_;

      TCPPeer& peer = peers_.emplace_back(sock, myId, address_);
      peer.Touch();

      delegate_.OnConnection(myId);
    }
  }

  // process peers
  for (auto peer = peers_.begin(); peer != peers_.end(); ++peer) {
    // we either killed the client or it timed out
    if (!peer->Open() || peer->HasDied()) {
      delegate_.OnDisconnection(peer->Id());

      peer = peers_.erase(peer);
      continue;
    }

    ssize_t n;
    while ((n = peer->sock_.read(chunkbuf_, sizeof(chunkbuf_))) > 0) {
      // still alive
      peer->Touch();

      delegate_.ProcessData(peer->Id(), chunkbuf_, n);
    }
  }

  while (auto* entry = outgoingQueue_.pop(&Entry::key)) {
    // broadcast
    if (entry->cid == kAllConnectId) {
      for (auto& p : peers_)
        p.Send(entry->data.get(), entry->dataSize);
      // send @ peer
    } else if (auto* peer = PeerById(entry->cid)) {
      peer->Send(entry->data.get(), entry->dataSize);
    }

    s_Pool.destruct(entry);
  }
}
}  // namespace network
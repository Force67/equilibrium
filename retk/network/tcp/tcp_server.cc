// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_server.h"
#include <network/util/sock_util.h>
#include <network/base/network_packet.h>

#include <ctime>

namespace network {

constexpr int kTCPKeepAliveSeconds = 45;

namespace {
uint32_t GetMartinSeed(void* noise) {
  uintptr_t address = reinterpret_cast<uintptr_t>(noise);
  uint32_t lower = static_cast<uint32_t>(address);

  lower += static_cast<uint32_t>(std::time(nullptr));
  return (lower << 16) | (lower >> 16);
}
}

struct TCPServer::Packet {
  connectid_t cid;
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Packet> key;
};

static base::object_pool<TCPServer::Packet> s_Pool;

TCPServer::TCPServer(TCPServerDelegate& delegate) : delegate_(delegate) {
  // 10/10 way of doing this..!
  _seed = GetMartinSeed(this);
}


int16_t TCPServer::TryHost(int16_t port) {
  _port = -1;

  for (int32_t i = 0; i < kDefaultPortRange; i++) {
    if (acceptor_.open(port)) {
      _port = port;
      break;
    }

    port++;
  }

  if (_port != -1) {
    acceptor_.set_non_blocking();
    util::SetTCPKeepAlive(acceptor_, true, kTCPKeepAliveSeconds);
  }

  return _port;
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

void TCPServer::Send(OpCode op, connectid_t id, const uint8_t* ptr, size_t len) {
  len += sizeof(PacketHeader);
  auto data = std::make_unique<uint8_t[]>(len);

  auto* header = reinterpret_cast<PacketHeader*>(data[0]);
  header->op = op;
  header->flags = 0;
  header->crc = 0;

  std::memcpy(data.get() + sizeof(PacketHeader), ptr, len);

  Packet* item = s_Pool.construct();
  item->cid = id;
  item->dataSize = static_cast<uint32_t>(len);
  item->data = std::move(data);
  queue_.push(&item->key);
}

void TCPServer::Broadcast(OpCode op, const uint8_t* ptr, size_t len) {
  Send(op, kAllConnectId, ptr, len);
}

void TCPServer::Tick() {
  // listen for incoming connections
  sockpp::tcp_socket sock = acceptor_.accept(&address_);
  if (sock.is_open()) {
    auto myId = ++_seed;

    TCPPeer& peer = peers_.emplace_back(sock, myId, address_);
    peer.Touch();

    delegate_.OnConnection(myId);
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
    while ((n = peer->sock_.read(workbuf_, sizeof(workbuf_))) > 0) {
      // still alive
      peer->Touch();

      delegate_.ProcessData(peer->Id(), workbuf_, n);
    }
  }

  while (auto* packet = queue_.pop(&Packet::key)) {
    // broadcast
    if (packet->cid == kAllConnectId) {
      for (auto& p : peers_)
        p.Send(packet->data.get(), packet->dataSize);
    // send @ peer
    } else if (auto* peer = PeerById(packet->cid)) {
      peer->Send(packet->data.get(), packet->dataSize);
    }

    s_Pool.destruct(packet);
  }
}
}  // namespace network
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_server.h"
#include <ctime>

namespace network {
namespace {
uint32_t GetMartinSeed(void* noise) {
  uintptr_t address = reinterpret_cast<uintptr_t>(noise);
  uint32_t lower = static_cast<uint32_t>(address);

  lower += static_cast<uint32_t>(std::time(nullptr));
  return (lower << 16) | (lower >> 16);
}
}

TCPServer::TCPServer(TCPServerDelegate& delegate) : delegate_(delegate) {
  // 10/10 way of doing this..!
  _seed = GetMartinSeed(this);
}


int16_t TCPServer::Host(int16_t port) {
  _port = -1;

  for (int32_t i = 0; i < kDefaultPortRange; i++) {
    if (_acc.open(port)) {
      _port = port;
      break;
    }

    port++;
  }

  if (_port != -1) {
    _acc.set_non_blocking();
    _acc.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);
  }

  return _port;
}

bool TCPServer::DropPeer(connectid_t cid) {
  if (TCPPeer* peer = PeerById(cid)) {
    peer->sock.close();

    return true;
  }

  return false;
}

void TCPServer::BroadcastPacket(const uint8_t* data,
                                size_t size,
                                connectid_t excluder /* = invalidid_t */) {
  for (auto& it : _peers) {
    if (it.id == excluder)
      continue;

    it.sock.write_n(static_cast<const void*>(data), size);
  }
}

TCPPeer* TCPServer::PeerById(connectid_t id) {
  auto it = std::find_if(_peers.begin(), _peers.end(),
                         [&](TCPPeer& it) { return it.id == id; });

  if (it == _peers.end())
    return nullptr;

  return &(*it);
}

void TCPServer::Tick() {
  // listen for incoming connections
  sockpp::tcp_socket sock = _acc.accept(&_addr);
  if (sock.is_open()) {
    TCPPeer& peer = _peers.emplace_back(sock);
    peer.id = ++_seed;
    peer.addr = _addr;

    delegate_.OnConnection(peer.id);
  }

  // process peers
  for (auto it = _peers.begin(); it != _peers.end(); ++it) {
    if (!it->open()) {
      delegate_.OnDisconnection(it->id);
      it = _peers.erase(it);
    } else {
      ssize_t n;

      while ((n = it->sock.read(workbuf_, sizeof(workbuf_))) > 0) {
        delegate_.ProcessData(it->id, workbuf_, n);
      }
    }
  }
}
}  // namespace network
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <network/tksp/tcp_server.h>
#include <network/util/sock_util.h>
#include <base/logging.h>

namespace network::tksp {

bool Server::Host(uint16_t port) {
  while (port <= (port + kPortRange)) {
    if (socket_.open(port))
      break;
  }

  if (socket_.is_open()) {
    my_address_ = {"localhost", port};

    if(!socket_.set_non_blocking() || !SetSocketOptions()) {
      socket_.reset();
      return false;
    }

    return true;
  }

  return false;
}

void Server::Quit() {
  for (auto& e : peer_list_) {
    PeerBase& peer = *(e);

    
  }
}

bool Server::SetSocketOptions() {
  util::SetTCPKeepAlive(socket_, true, kKeepAliveCount);
}

bool Server::Process() {
  // listen for incoming connections
  sockpp::tcp_socket sock = socket_.accept(&next_address_);
  if (sock) {
    auto nextId = ++seed_;
    auto peer =
        std::make_unique<PeerBase>(std::move(sock), nextId, next_address_);

    peer->Touch();
    peer_list_.push_back(std::move(peer));

    // this is bullshit
    JoinRequest request{kProtocolVersion};
    delegate_.OnConnection(nextId, next_address_, request);
  }

  Host::Process();


}

bool TCPServer::DropPeer(PeerId cid) {
  if (NetworkPeer* peer = PeerById(cid)) {
    // trigger the disconnection event on the next frame
    peer->Kill();
    peer->Close();
    return true;
  }

  return false;
}

void TCPServer::BroadcastPacket(const uint8_t* data,
                                size_t size,
                                PeerId excluder /* = invalidid_t */) {
  for (auto& it : peers_) {
    if (it->ConnectId() == excluder)
      continue;

    it->Send(data, size);
  }
}

NetworkPeer* TCPServer::PeerById(PeerId id) {
  auto it = std::find_if(peers_.begin(), peers_.end(),
                         [&](auto& it) { return (*it).ConnectId() == id; });

  if (it == peers_.end())
    return nullptr;

  return (*it).get();
}

}  // namespace network
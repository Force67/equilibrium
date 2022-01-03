// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#if 0
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
  NotifyQuit notification{NotifyQuit::Reason::kQuit};
  QueueBroadcast(Chunkheader::Type::kBye, notification);

  delegate_.OnDisconnection(PeerBase::kAllConnectId, notification);
  // TODO: set process timer, to timeout after sending msg.
}

bool Server::SetSocketOptions() {
  return util::SetTCPKeepAlive(socket_, true, kKeepAliveCount);
}

bool Server::Process() {
  // listen for incoming connections

  // THIS needs to be moved
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

  return true;
}

}  // namespace network
#endif
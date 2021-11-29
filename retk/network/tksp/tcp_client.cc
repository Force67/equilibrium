// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#if 0
#include "tcp_client.h"
#include <base/container/object_pool.h>
#include <base/logging.h>
#include <network/util/sock_util.h>
#include <network/tksp/tcp_client.h>

using namespace std::chrono_literals;

namespace network::tksp {

bool Client::Connect(const PeerBase::Adress& remote_address) {
  bool result;
  result = socket_.connect(remote_address);
  result = socket_.set_non_blocking(true);
  result = SetSocketOptions();
  if (!result) {
    socket_.reset();
    return false;
  }


  TK_DCHECK(!peer_list_.empty());

  // hacky workaround thing:
  //peer_list_.push_back(std::make_unique<PeerBase>());

  JoinRequest command{kProtocolVersion};
  QueueOutgoingCommand(Chunkheader::Type::kHello, command);

  delegate_.OnConnection(PeerBase::kAllConnectId, MainPeer().address, command);
  last_ping_ = util::msec();

  return result;
}

void Client::Disconnect() {
  NotifyQuit notification{NotifyQuit::Reason::kQuit};
  QueueOutgoingCommand(Chunkheader::Type::kBye, notification);

  // allow up to 3 seconds to send any requests
  // this is not so bueno since we might have this executed on the wrong thread
  // :(
  util::MilliSeconds timer = util::msec();
  while (timer <= timer + 3s) {
    Process();
  }

  delegate_.OnDisconnection(PeerBase::kAllConnectId, notification);
  socket_.close();
}

bool Client::SetSocketOptions() {
  bool result;
  result = result = socket_.read_timeout(0ms);
  result = socket_.write_timeout(0ms);
  result = util::SetTCPKeepAlive(socket_, true, kKeepAliveCount);
  return result;
}

bool Client::Process() {
  if (!socket_.is_connected()) {
    NotifyQuit notification{NotifyQuit::Reason::kConnectionLost};
    delegate_.OnDisconnection(PeerBase::kAllConnectId, notification);
    return false;
  }

  // receive latest commands
  Host::Process();

  // send activity ack
  if ((util::msec() - last_ping_) >= kPingRate) {
    Host::QueueOutgoingCommand(Chunkheader::Type::kPing, MainPeer().id, nullptr,
                               0);
    last_ping_ = util::msec();
  }
}

bool Client::Connected() const {
  return socket_.is_connected();
}

PeerBase& Client::MainPeer() {
  TK_DCHECK(IsClient());

  return (*peer_list_[0]);
}
}  // namespace network::tksp
#endif
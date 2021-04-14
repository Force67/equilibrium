// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "tcp_client.h"
#include <base/object_pool.h>
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

  // TODO: add me as peer.!

  JoinRequest command{kProtocolVersion};
  QueueOutgoingCommand(CommandId::kIJoin, command);

  delegate_.OnConnection(PeerBase::kAllConnectId, MainPeer().address, command);
  last_ping_ = util::msec();

  return result;
}

void Client::Disconnect() {
  QuitCommand command{QuitReason::kIWantToQuit};
  QueueOutgoingCommand(CommandId::kIQuit, command);

  // allow up to 3 seconds to send any requests
  util::MilliSeconds timer = util::msec();
  while (timer <= timer + 3s) {
    Process();
  }

  NotifyQuit notification{NotifyQuit::Reason::kQuit};
  delegate_.OnDisconnection(PeerBase::kAllConnectId, notification);
  socket_.close();
}

bool Client::SetSocketOptions() {
  bool result;
  result = 
  result = socket_.read_timeout(0ms);
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
    QueueCommand(CommandId::kPing, nullptr, 0);
    last_ping_ = util::msec();
  }
}

bool Client::Connected() const {
  return socket_.is_connected();
}

PeerBase& Client::MainPeer() {
  LOG_DCHECK(IsClient());

  return (*peer_list_[0]);
}

}
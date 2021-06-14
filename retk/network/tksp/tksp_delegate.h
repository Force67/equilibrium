// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/tksp/tksp_protocol.h>
#include <network/base/peer_base.h>

namespace network::tksp {

// implementation class for reacting to host events
class TkspDelegate {
 public:
  virtual ~TkspDelegate() = default;

  // called when a new connection is received
  virtual void OnConnection(PeerBase::Id id, const PeerBase::Adress& address, const JoinRequest& request){};

  // called when a connection quit is received
  virtual void OnDisconnection(PeerBase::Id id, const NotifyQuit& notification) = 0;

  // called when new data is received
  // note that the provided length doesn't include the protocol header size
  // since that is considered an internal protocol detail
  virtual void ProcessData(PeerBase::Id id, const uint8_t* data, size_t length) = 0;
};
}
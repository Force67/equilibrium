// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_dispatcher.h"
#include "zeta_connection.h"

namespace network {

ZetaDispatcher::ZetaDispatcher() {}

ZetaDispatcher::~ZetaDispatcher() {}

void ZetaDispatcher::Shutdown() {
  for (auto& it : peer_map_) {
    ZetaConnection* connection = it.second.get();
    connection->CloseConnection();
  }

  // delete peers.
  peer_map_.clear();
}

void ZetaDispatcher::ProcessPacket(FrameType frame_type,
                                   PeerBase::Id peer_id,
                                   const uint8_t* buffer,
                                   size_t length) {
  auto it = peer_map_.find(peer_id);
  if (it != peer_map_.end()) {
    // send directly to connection, which will handle message events, 
    // and dispatch to network stack.
    ZetaConnection* connection = it->second.get();
    connection->ProcessCommand(frame_type, buffer, length);
  }
}
}  // namespace network
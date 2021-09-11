// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_server.h"
#include "zeta_connection.h"

namespace network {

ZetaServer::ZetaServer() {
  packet_reader_ = std::make_unique<ZetaPacketReader>(&dispatcher_);
}

ZetaServer::~ZetaServer() {}

bool ZetaServer::TickFrame() {
  for (const ZetaDispatcher::PeerEntry& obj : dispatcher_.peer_map()) {
    packet_reader_->ReadAndDispatch(*obj.second);
  
  }

  return true;
}
}  // namespace network
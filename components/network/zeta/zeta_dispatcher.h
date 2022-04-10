// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// The dispatcher is responsible for parsing and dispatching the packets
// on the server side.
#pragma once

#include <memory>
#include <unordered_map>
#include <network/base/peer_base.h>
#include <network/zeta/zeta_protocol.h>
#include <network/zeta/zeta_packet_writer.h>

namespace network {
class ZetaConnection;

class ZetaDispatcher {
 public:
  ZetaDispatcher();
  ~ZetaDispatcher();

  void ProcessPacket(FrameType type, PeerBase::Id peer_id, const uint8_t* buffer, size_t length);

  void Shutdown();

  // todo: session or connection?
  using PeerMap =
      std::unordered_map<PeerBase::Id, std::unique_ptr<ZetaConnection>>;

  using PeerEntry = PeerMap::value_type;

  const PeerMap& peer_map() { return peer_map_; }

 private:
  // responsible for dumping the data to the socket level
  std::unique_ptr<ZetaPacketWriter> writer_;

  PeerMap peer_map_;
};
}  // namespace network
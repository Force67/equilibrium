// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/zeta/zeta_connection.h>
#include <network/zeta/zeta_dispatcher.h>
#include <network/zeta/zeta_packet_reader.h>

// https://source.chromium.org/chromium/chromium/src/+/main:net/quic/quic_chromium_packet_reader.h;l=53;drc=b6d364a7c841ce51a9bc2d2a32a84ce5787e63eb;bpv=1;bpt=1?q=StartReading&ss=chromium%2Fchromium%2Fsrc

namespace network {
class ZetaClient final {
 public:
  ZetaClient();

  bool Connect(const PeerBase::Adress&);

 private:
  ZetaDispatcher dispatcher_;
  std::unique_ptr<ZetaPacketReader> packet_reader_;
  std::unique_ptr<ZetaConnection> connection_;
};
}  // namespace network

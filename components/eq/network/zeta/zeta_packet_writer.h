// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace network {

class ZetaPacketWriter {
 public:
  bool WritePacket(PeerBase &target, const uint8_t* buf, size_t length);

  // flush (if underlying data is queued), else no-op
  void Flush();
private:

};
}
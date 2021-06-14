// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace network {

class ZetaPacketWriter {
 public:
  // writes the data immediately to a peer's socket
  virtual bool WritePacket(const uint8_t* buf, size_t length);

  // flush (if underlying data is queued), else no-op
  virtual void Flush();
};
}
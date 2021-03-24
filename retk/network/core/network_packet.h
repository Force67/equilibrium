// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include "network_base.h"

namespace network {

enum class OpCode {
  // system ops
  kQuit = 0xb9d273f2,
  kJoin = 0x781aa8ec,
  kHeartbeat = 0x7440ad27,

  // user ops
  kData = 0xb36fcb5c
};

enum class OpStatus {
};

struct PacketHeader {
  OpCode op;
  uint32_t flags;
  uint32_t crc;
};

struct Packet {
  PacketHeader header;
  uint32_t dataSize;
  std::unique_ptr<uint8_t> data;
};
}
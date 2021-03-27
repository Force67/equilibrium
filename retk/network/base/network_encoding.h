// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace network {

// version of the system encoding (this)
// of the network stack
constexpr uint32_t kEncodingVersion = 1;

// if the to be sent message is over kChunkSize
// it must be split.
constexpr uint32_t kChunkSize = 2048;

// identifier for a peer. 
using connectid_t = uint32_t;

enum class CommandId : uint32_t {
  // system commands
  kIQuit = 0xb9d273f2, // QuitCommand
  kIJoin = 0x781aa8ec, // JoinCommand
  kIPing = 0x7440ad27, // No data
  // user commands
  kData = 0xb36fcb5c
};

struct JoinCommand {
  uint32_t encodingVersion;
};

enum class QuitReason : uint16_t {
	kIWantToQuit,
	kIGotKicked,
	kBrokenProtocol
};

struct QuitCommand {
  QuitReason reason;
};

struct Chunkheader {
  CommandId id;
  uint32_t crc;
};
}
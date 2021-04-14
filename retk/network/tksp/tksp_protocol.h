// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/base/peer_base.h>

namespace network::tksp {

constexpr uint32_t kProtocolVersion = 11;

#pragma pack(push, 8)

// a chunk contains the chunk-header, followed by command data
// messages of the type kData will be followed by user data instead

struct Chunkheader {
  enum class Type : uint32_t {
    kInvalid = 0,

    kHello,
    kBye,
    kPing,

    // user data
    kPayload,

    kNumMessages
  };
  static const char* TypeToString(Type);

  Chunkheader() = default;
  explicit Chunkheader(Type t, uint32_t size) : type(t), size(size) {}

  // max size of a chunk
  static constexpr uint32_t kMaxSize = 2048;

  // size includes header
  uint32_t size = 0;
  Type type = Type::kInvalid;

  // assigned by sender, and kept track of
  // by the server
  uint32_t transaction_id = 0;
};

struct JoinRequest {
  uint32_t protocol_version;
};
struct JoinReply {
  enum class Response { kDeny, kAllow };
};

struct NotifyQuit {
  enum class Reason : uint16_t {
    kQuit,
    kTimeout,
    kConnectionLost,
  } reason{Reason::kQuit};
};

#pragma pack(pop)
}  // namespace network::tksp
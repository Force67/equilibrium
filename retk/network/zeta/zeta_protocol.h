// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/base/peer_base.h>

namespace network {

// global protocol version.
constexpr uint32_t kProtocolVersion = 11;

#pragma pack(push, 8)

enum class FrameType : uint32_t {
  kInvalid = 0,

  kHello,
  kBye,
  kPing,

  // user data
  kPayload,

  kNumMessages,
};

static const char* FrameTypeToString(FrameType);

// Frames contain the header, followed by command data
struct FrameHeader {
  // max size of a frame
  static constexpr uint32_t kMaxSize = 2048;

  // magic constant
  static constexpr uint32_t kMagic = 0x80fb0831;

  FrameHeader() = default;
  explicit FrameHeader(FrameType t, uint32_t size) : type(t), size(size) {}

  // magic to identify a Z packet
  uint32_t magic = kMagic;

  // size includes header
  uint32_t size = 0;
  FrameType type = FrameType::kInvalid;

  // assigned by sender, and kept track of
  // by the server
  uint32_t transaction_id = 0;
};

static constexpr size_t kMinFrameSize = sizeof(FrameHeader);

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
}  // namespace network
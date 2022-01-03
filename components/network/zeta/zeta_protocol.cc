// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <network/zeta/zeta_protocol.h>

namespace network {

const char* FrameTypeToString(FrameType type) {
  switch (type) {
    case FrameType::kInvalid:
      return "Invalid";
    case FrameType::kHello:
      return "Hello";
    case FrameType::kBye:
      return "Bye";
    case FrameType::kPing:
      return "Ping";
    case FrameType::kPayload:
      return "Payload";
    default:
      IMPOSSIBLE;
      return "<invalid>";
  }
}
}  // namespace network
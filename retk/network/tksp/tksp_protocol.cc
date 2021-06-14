// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <network/tksp/tksp_protocol.h>

namespace network::tksp {

const char* Chunkheader::TypeToString(Type type) {
  switch (type) {
    case Type::kInvalid:
      return "Invalid";
    case Type::kHello:
      return "Hello";
    case Type::kBye:
      return "Bye";
    case Type::kPing:
      return "Ping";
    case Type::kPayload:
      return "Payload";
    default:
      TK_IMPOSSIBLE;
      return "<invalid>";
  }
}
}  // namespace network::tksp
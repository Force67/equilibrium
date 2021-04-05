// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <flatbuffers/flatbuffers.h>
#include <sync/protocol/generated/message_root_generated.h>

namespace protocol {
struct MessageRoot;
}

namespace sync {

constexpr int kDefaultSyncPort = 4523;
constexpr char kDefaultSyncIp[] = "localhost";

using FbsBuffer = flatbuffers::FlatBufferBuilder;

// Flat buffer aliases
template <typename T>
using FbsRef = flatbuffers::Offset<T>;
using FbsStringRef = flatbuffers::Offset<flatbuffers::String>;

inline const protocol::MessageRoot* UnpackMessage(const uint8_t* data,
                                                  size_t length) noexcept {
  flatbuffers::Verifier verifier(data, length);
  if (!protocol::VerifyMessageRootBuffer(verifier))
    return nullptr;

  const protocol::MessageRoot* root =
      protocol::GetMessageRoot(static_cast<const void*>(data));
  return root;
}

}  // namespace sync
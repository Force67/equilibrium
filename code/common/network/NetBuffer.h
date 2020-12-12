// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"
#include "protocol/generated/MessageRoot_generated.h"

namespace network {
  namespace pt = protocol;

  // TODO: discard this when netbuffer is done.
  using FbsBuffer = flatbuffers::FlatBufferBuilder;

  // Flat buffer aliases
  template <typename T>
  using FbsRef = flatbuffers::Offset<T>;
  using FbsStringRef = flatbuffers::Offset<flatbuffers::String>;

  class NetBuffer final : public flatbuffers::FlatBufferBuilder {
  public:
	NetBuffer();
	~NetBuffer();
  };
} // namespace network
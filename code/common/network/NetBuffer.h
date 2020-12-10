// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Netbase.h"

namespace network {

class NetBuffer final : public flatbuffers::FlatBufferBuilder {
public:
  NetBuffer();
  ~NetBuffer();


};
}
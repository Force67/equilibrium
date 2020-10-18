// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "NetLib.h"

struct ENetPeer;

namespace netlib {

  class PeerBase {
  public:
  private:
	ENetPeer *peer;
  };
} // namespace netlib
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <enet/enet.h>

namespace noda {

  struct Client {
	ENetPeer *peer;
  };
} // namespace noda
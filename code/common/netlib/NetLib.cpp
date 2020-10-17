// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NetLib.h"

#include <cassert>
#include <enet/enet.h>

namespace netlib {

  void InitNetLib()
  {
	assert(enet_initialize() == 0);
  }

  void DestroyNetLib()
  {
	enet_deinitialize();
  }
} // namespace netlib
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "Sigmaker.h"
#include "utils/Logger.h"

namespace features {

  void GenerateDumbSig()
  {
	size_t opCount = 9999;
	const ea_t address = get_screen_ea();

	if(address == BADADDR) {
	  LOG_ERROR("You must select an address");
	}

	LOG_TRACE("Creating Signature...");
  }
} // namespace features
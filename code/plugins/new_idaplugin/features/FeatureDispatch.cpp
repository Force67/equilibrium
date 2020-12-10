// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "FeatureDispatch.h"
#include "Sigmaker.h"

#include "utils/Logger.h"

namespace features {

  void DispatchFeatureIndex(FeatureIndex index)
  {
	switch(index) {
	case FeatureIndex::DumbSig:
	  GenerateDumbSig();
	  break;
	default:
	  LOG_ERROR("Unable to find feature index");
	}
  }
} // namespace features
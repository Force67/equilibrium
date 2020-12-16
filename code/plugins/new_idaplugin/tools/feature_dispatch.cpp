// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "FeatureDispatch.h"
#include "Pch.h"
#include "sigmaker/Sigmaker.h"

#include "utils/Logger.h"

namespace features {

void DispatchFeatureIndex(FeatureIndex index) {
  switch (index) {
    case FeatureIndex::DumbSig:
      GenerateSignature();
      break;
    default:
      LOG_ERROR("Unable to find feature index");
  }
}
}  // namespace features
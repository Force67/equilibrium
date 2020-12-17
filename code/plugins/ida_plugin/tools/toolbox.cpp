// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "toolbox.h"
#include "utils/Opt.h"
#include "utils/logger.h"

#include "signature/sigmaker.h"

namespace {
// Somethin
utils::Opt<bool> ToolCache{false, "Nd_PatternCache"};
}  // namespace

// s_mToolBox;


namespace tools {

Toolbox::Toolbox() {}
Toolbox::~Toolbox() {}

void Toolbox::RegisterPattern(const std::string&) {}

void Toolbox::TriggerFeature(FeatureCode code) {
  switch (code) {
    case FeatureCode::kSignature:
      tools::GenerateSignature();
      break;
    default:
      LOG_ERROR("Unable to find feature index");
  }
}
}
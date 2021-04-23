// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "toolbox.h"
#include "utils/Opt.h"

#include "bindings/api_binding.h"
#include "signature/sigmaker.h"

namespace {
// Somethin
utils::Opt<bool> ToolCache{false, "Nd_PatternCache"};
}  // namespace

// s_mToolBox;


namespace tools {

Toolbox::Toolbox() {
  binding::HACK_StaticRegisterBindings();
}
Toolbox::~Toolbox() {
  binding::HACK_StaticRemoveBindings();
}

void Toolbox::RegisterPattern(const std::string&) {}

void Toolbox::TriggerFeature(FeatureCode code) {
  switch (code) {
    case FeatureCode::kSignature:
      tools::GenerateSignature(get_screen_ea());
      break;
    default:
      LOG_ERROR("Unable to find feature index");
  }
}
}
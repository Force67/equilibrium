// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "toolbox.h"
#include "utils/Opt.h"

#include "bindings/api_binding.h"
#include "signature_generator.h"

namespace tools {
namespace {
static Toolbox* s_ToolBox = nullptr;
}

Toolbox* toolbox() noexcept {
  // TODO: dcheck this value!
  return s_ToolBox;
}

Toolbox::Toolbox() : generator_(this) {
  s_ToolBox = this;
  binding::HACK_StaticRegisterBindings();
}
Toolbox::~Toolbox() {
  binding::HACK_StaticRemoveBindings();
  s_ToolBox = nullptr;
}

void Toolbox::RegisterPattern(const std::string&) {}

void Toolbox::InvokeAction(ActionCode code) {
  switch (code) {
    case ActionCode::kSignature: {
      generator_.UniquePattern(get_screen_ea(), false);
      break;
    }
    default:
      LOG_ERROR("Unable to find feature index");
  }
}
}  // namespace tools
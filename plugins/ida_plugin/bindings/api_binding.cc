// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <expr.hpp>
#include "api_binding.h"

#include "api_toolbox.h"

namespace binding {

APIBinding::APIBinding() {
}

APIBinding::~APIBinding() {
}

void HACK_StaticRegisterBindings() {
  RegisterToolboxApi();
}

void HACK_StaticRemoveBindings() {
  DestroyToolboxApi();
}

}
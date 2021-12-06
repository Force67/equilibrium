// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#include "base/check.h"

namespace base {
static AssertHandler s_handler{};

void SetAssertHandler(AssertHandler handler) {
  s_handler = handler;
}

void InvokeAssertHandler() {
  s_handler();
}
}  // namespace base

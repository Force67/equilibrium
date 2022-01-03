// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "context_holder.h"

namespace network {

static bool s_NetInitialized = false;

ContextHolder::ContextHolder() {
  if (!s_NetInitialized) {
    sockpp::socket::initialize();
    s_NetInitialized = true;
  }
}

ContextHolder::~ContextHolder() {
  if (s_NetInitialized) {
    sockpp::socket::destroy();
    s_NetInitialized = false;
  }
}

bool ContextHolder::Initialized() {
  return s_NetInitialized;
}
}  // namespace network
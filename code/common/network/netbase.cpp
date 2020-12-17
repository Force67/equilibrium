// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "netbase.h"

namespace network {

static bool s_NetInitialized = false;

Context::Context() {
  if (!s_NetInitialized) {
    sockpp::socket::initialize();
    s_NetInitialized = true;
  }
}

Context::~Context() {
  if (s_NetInitialized) {
    sockpp::socket::destroy();
    s_NetInitialized = false;
  }
}

bool Context::Initialized() {
  return s_NetInitialized;
}
}  // namespace network
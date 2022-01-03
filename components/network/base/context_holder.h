// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/socket.h>
#include <limits>

namespace network {
// Required to be created before using any network functions
struct ContextHolder {
  ContextHolder();
  ~ContextHolder();

  static bool Initialized();
};
}  // namespace network
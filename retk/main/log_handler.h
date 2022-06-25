// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/logging.h>

namespace main {

class LogHandler {
 public:
  LogHandler();
  ~LogHandler();

  void BuildMessage(base::LogLevel, const char* msg);
};
}  // namespace main

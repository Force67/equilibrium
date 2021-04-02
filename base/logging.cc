// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "logging.h"

namespace base {

const char* const kLevelToNames[] = {"trace", "debug", "info", "warning",
                                     "error"};

const char* LevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

static LogHandler s_callback{nullptr};

void PrintLogMessage(LogLevel ll,
                     const char* text,
                     const fmt::format_args& args) {
  auto fmt = fmt::vformat(text, args);

  if (s_callback)
    s_callback(ll, fmt.c_str());
}

void InitLogging(LogHandler callback) {
  s_callback = callback;
}
}  // namespace base
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>
#include "Server.h"

namespace sync_server::utils {

void PrintLogMessageImpl(LogLevel, const char*, const fmt::format_args&);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  PrintLogMessageImpl(level, format, fmt::make_format_args(args...));
}

void SetLogCallback(logcallback_t);
}  // namespace sync_server::utils
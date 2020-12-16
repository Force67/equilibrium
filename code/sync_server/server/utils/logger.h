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

#ifdef ND_DEBUG
#define LOG_TRACE(...)                                                   \
  ::sync_server::utils::PrintLogMessage(::sync_server::LogLevel::kTrace, \
                                        __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif
#define LOG_INFO(...)                                                   \
  ::sync_server::utils::PrintLogMessage(::sync_server::LogLevel::kInfo, \
                                        __VA_ARGS__)
#define LOG_WARNING(...)                                                   \
  ::sync_server::utils::PrintLogMessage(::sync_server::LogLevel::kWarning, \
                                        __VA_ARGS__)
#define LOG_ERROR(...)                                                   \
  ::sync_server::utils::PrintLogMessage(::sync_server::LogLevel::kError, \
                                        __VA_ARGS__)

void SetLogCallback(logcallback_t);
}  // namespace sync_server::utils
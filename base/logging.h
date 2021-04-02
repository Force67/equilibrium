// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>

namespace base {

enum class LogLevel { kTrace, kDebug, kInfo, kWarning, kError, kAll };

const char* LevelToName(LogLevel level);

using LogHandler = void (*)(LogLevel, const char*);

void InitLogging(LogHandler);

void PrintLogMessage(LogLevel, const char*, const fmt::format_args&);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  PrintLogMessage(level, format, fmt::make_format_args(args...));
}
}  // namespace base

#ifdef ND_DEBUG
#define LOG_TRACE(...)                                                   \
  ::base::PrintLogMessage(::sync_server::LogLevel::kTrace, \
                                        __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif
#define LOG_INFO(...)                                                   \
  ::base::PrintLogMessage(::sync_server::LogLevel::kInfo, \
                                        __VA_ARGS__)
#define LOG_WARNING(...)                                                   \
  ::base::PrintLogMessage(::sync_server::LogLevel::kWarning, \
                                        __VA_ARGS__)
#define LOG_ERROR(...)                                                   \
  ::base::PrintLogMessage(::sync_server::LogLevel::kError, \
                                        __VA_ARGS__)

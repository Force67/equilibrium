// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>
#include <string_view>

namespace noda {
enum class LogLevel { Trace, Info, Warning, Error };

void PrintLogMessageImpl(LogLevel, const char*, const fmt::format_args&);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  PrintLogMessageImpl(level, format, fmt::make_format_args(args...));
}

#if _DEBUG
#define LOG_TRACE(...) \
  ::noda::PrintLogMessage(::noda::LogLevel::Trace, __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif
#define LOG_INFO(...) \
  ::noda::PrintLogMessage(::noda::LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) \
  ::noda::PrintLogMessage(::noda::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) \
  ::noda::PrintLogMessage(::noda::LogLevel::Error, __VA_ARGS__)
}  // namespace noda
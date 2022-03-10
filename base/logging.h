// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>

// TODO: support concept of spaces
// TODO: support channels
namespace base {

enum class LogLevel : uint32_t {
  kTrace,
  kDebug,
  kInfo,
  kWarning,
  kError,
  kFatal,
  kAll
};
const char* LevelToName(LogLevel level) noexcept;

using LogHandler = void (*)(LogLevel, const char*);
// Init the global log device
void SetLogHandler(LogHandler);
// Print the copyright message to the device
void PrintLegals();

void Core_PrintLogMessage(LogLevel, const char*, const fmt::format_args&);
void Core_PrintLogMessage(LogLevel, const char*);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  Core_PrintLogMessage(level, format, fmt::make_format_args(args...));
}

// adadpter function for legacy printf style systems
// please use the LOG_X macros
void PrintLogMessagePF(LogLevel level, const char* format...);
}  // namespace base

#if defined(TK_DBG)
#define LOG_DEBUG(...) \
  ::base::PrintLogMessage(::base::LogLevel::kDebug, __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif
#define LOG_TRACE(...) \
  ::base::PrintLogMessage(::base::LogLevel::kTrace, __VA_ARGS__)
#define LOG_INFO(...) \
  ::base::PrintLogMessage(::base::LogLevel::kInfo, __VA_ARGS__)
#define LOG_WARNING(...) \
  ::base::PrintLogMessage(::base::LogLevel::kWarning, __VA_ARGS__)
#define LOG_ERROR(...) \
  ::base::PrintLogMessage(::base::LogLevel::kError, __VA_ARGS__)
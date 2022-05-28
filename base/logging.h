// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>
#include <base/compiler.h>

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

namespace detail {
void WriteLogMessage(LogLevel, const char*, const fmt::format_args&);
void WriteLogMessage(LogLevel, const char*);
}  // namespace detail

using LogHandler = void (*)(void*, LogLevel, const char*);
void SetLogHandler(LogHandler, void* user_pointer);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  detail::WriteLogMessage(level, format, fmt::make_format_args(args...));
}

// adadpter function for legacy printf style systems
// please use the LOG_X macros
void PrintLogMessagePF(LogLevel level, const char* format...);
}  // namespace base

#if defined(CONFIG_DEBUG)
// debug = trace
#define LOG_DEBUG(...) ::base::PrintLogMessage(::base::LogLevel::kDebug, __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif
#define LOG_TRACE(...) ::base::PrintLogMessage(::base::LogLevel::kTrace, __VA_ARGS__)
#define LOG_INFO(...) ::base::PrintLogMessage(::base::LogLevel::kInfo, __VA_ARGS__)
#define LOG_WARNING(...) \
  ::base::PrintLogMessage(::base::LogLevel::kWarning, __VA_ARGS__)
#define LOG_ERROR(...) ::base::PrintLogMessage(::base::LogLevel::kError, __VA_ARGS__)
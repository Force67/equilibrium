// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#define FMT_ENABLE_WIDE
#include <fmt/xchar.h>
#include <fmt/printf.h>

#include <base/export.h>
#include <base/compiler.h>

namespace base {

enum class LogLevel { kTrace, kDebug, kInfo, kWarning, kError, kFatal, kAll };
const char* LogLevelToName(LogLevel level) noexcept;

namespace detail {
BASE_EXPORT void WriteLogMessage(const char* channel_name,
                                 LogLevel,
                                 const char*,
                                 const fmt::format_args&);
BASE_EXPORT void WriteLogMessagef(const char* channel_name,
                                  LogLevel,
                                  const char*,
                                  const fmt::basic_format_args<fmt::printf_context>&);
BASE_EXPORT void WriteLogMessage(const char* channel_name, LogLevel, const char*);
}  // namespace detail

using LogHandler = void (*)(void*, const char* channel_name, LogLevel, const char*);
BASE_EXPORT void SetLogHandler(LogHandler, void* user_pointer) noexcept;
BASE_EXPORT void SetLogInstance(void* user_pointer);

template <typename... Args>
void PrintLogMessage(const char* channel_name,
                     LogLevel level,
                     const char* format,
                     const Args&... args) {
  detail::WriteLogMessage(channel_name, level, format, fmt::make_format_args(args...));
}

// adadpter function for legacy printf style systems
template <typename... Args>
void PrintfLogMessage(const char* channel_name,
                      LogLevel level,
                      const char* format,
                      const Args&... args) {
  using context = fmt::basic_printf_context<char>;
  detail::WriteLogMessagef(channel_name, level, format,
                           fmt::make_format_args<context>(args...));
}
}  // namespace base

// old log api..
#if defined(CONFIG_DEBUG)
#define BASE_LOG_DEBUG(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kDebug, __VA_ARGS__)
#define BASE_LOG_TRACE(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kTrace, __VA_ARGS__)
// >>> new log api
#define BASE_LOGD(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kDebug, __VA_ARGS__)
#define BASE_LOGT(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kTrace, __VA_ARGS__)
// <<< new log api
#else
#define BASE_LOG_DEBUG(...)
#define BASE_LOG_TRACE(...)
#define BASE_LOGD(...)
#define BASE_LOGT(...)
#endif

#define BASE_LOG_INFO(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kInfo, __VA_ARGS__)
#define BASE_LOG_WARNING(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kWarning, __VA_ARGS__)
#define BASE_LOG_ERROR(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kError, __VA_ARGS__)
#define BASE_LOG_FATAL(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kFatal, __VA_ARGS__)

// >>> new log api
#define BASE_LOGI(c, ...) ::base::PrintLogMessage(c, ::base::LogLevel::kInfo, __VA_ARGS__)
#define BASE_LOGW(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kWarning, __VA_ARGS__)
#define BASE_LOGE(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kError, __VA_ARGS__)
#define BASE_LOGF(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kFatal, __VA_ARGS__)
// <<< new log api

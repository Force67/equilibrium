// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/compiler.h>
#include <base/strings/format.h>

namespace base {

enum class LogLevel { kTrace, kDebug, kInfo, kWarning, kError, kFatal, kAll };
BASE_EXPORT const char* LogLevelToName(LogLevel level) noexcept;

namespace detail {
BASE_EXPORT void WriteLogMessage(const char* channel_name, LogLevel, const char*);
}  // namespace detail

using LogHandler = void (*)(void*, const char* channel_name, LogLevel, const char*);
BASE_EXPORT void SetLogHandler(LogHandler, void* user_pointer) noexcept;
BASE_EXPORT void SetLogInstance(void* user_pointer);

// Per-channel and global log level filtering.
BASE_EXPORT void SetChannelMinLevel(const char* channel_name,
                                    LogLevel min_level) noexcept;
BASE_EXPORT void SetGlobalMinLevel(LogLevel min_level) noexcept;
BASE_EXPORT bool ShouldLog(const char* channel_name, LogLevel level) noexcept;

// Plain string overload (no formatting).
inline void PrintLogMessage(const char* channel_name,
                            LogLevel level,
                            const char* message) {
  detail::WriteLogMessage(channel_name, level, message);
}

// {}-style formatted logging via base::FormatTo. The common case fits in
// the stack buffer; oversized messages spill into a heap String.
template <typename... Args>
void PrintLogMessage(const char* channel_name,
                     LogLevel level,
                     const char* format,
                     const Args&... args) {
  if (!ShouldLog(channel_name, level)) return;
  char stack_buf[1024];
  mem_size needed = base::FormatTo(stack_buf, sizeof(stack_buf), format, args...);
  if (needed < sizeof(stack_buf)) {
    detail::WriteLogMessage(channel_name, level, stack_buf);
    return;
  }
  base::String big;
  big.reserve(static_cast<base::String::size_type>(needed + 1));
  base::FormatTo(big, format, args...);
  detail::WriteLogMessage(channel_name, level, big.c_str());
}

}  // namespace base

#if defined(CONFIG_DEBUG)
#define BASE_LOG_DEBUG(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kDebug, __VA_ARGS__)
#define BASE_LOG_TRACE(...) \
  ::base::PrintLogMessage(PROJECT_NAME, ::base::LogLevel::kTrace, __VA_ARGS__)
#define BASE_LOGD(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kDebug, __VA_ARGS__)
#define BASE_LOGT(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kTrace, __VA_ARGS__)
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

#define BASE_LOGI(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kInfo, __VA_ARGS__)
#define BASE_LOGW(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kWarning, __VA_ARGS__)
#define BASE_LOGE(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kError, __VA_ARGS__)
#define BASE_LOGF(c, ...) \
  ::base::PrintLogMessage(c, ::base::LogLevel::kFatal, __VA_ARGS__)

// Disable all logging when requested.
#ifdef DISABLE_BASE_LOG
#undef BASE_LOG_INFO
#undef BASE_LOG_WARNING
#undef BASE_LOG_ERROR
#undef BASE_LOG_FATAL
#undef BASE_LOG_DEBUG
#undef BASE_LOG_TRACE
#undef BASE_LOGI
#undef BASE_LOGW
#undef BASE_LOGE
#undef BASE_LOGF
#undef BASE_LOGD
#undef BASE_LOGT
#define BASE_LOG_INFO(...)
#define BASE_LOG_WARNING(...)
#define BASE_LOG_ERROR(...)
#define BASE_LOG_FATAL(...)
#define BASE_LOG_DEBUG(...)
#define BASE_LOG_TRACE(...)
#define BASE_LOGI(...)
#define BASE_LOGW(...)
#define BASE_LOGE(...)
#define BASE_LOGF(...)
#define BASE_LOGD(...)
#define BASE_LOGT(...)
#endif

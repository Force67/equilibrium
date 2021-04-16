// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>

namespace base {

enum class LogLevel { kTrace, kDebug, kInfo, kWarning, kError, kAll };

const char* LevelToName(LogLevel level) noexcept;

using LogHandler = void (*)(LogLevel, const char*);

void InitLogging(LogHandler);

void Core_PrintLogMessage(LogLevel, const char*, const fmt::format_args&);

template <typename... Args>
void PrintLogMessage(LogLevel level, const char* format, const Args&... args) {
  Core_PrintLogMessage(level, format, fmt::make_format_args(args...));
}
}  // namespace base

#ifndef __FUNCTION_NAME__
#ifdef WIN32  // WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else  //*NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

#if defined(TK_DBG)
#define LOG_TRACE(...) \
  ::base::PrintLogMessage(::base::LogLevel::kTrace, __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif
#define LOG_INFO(...) \
  ::base::PrintLogMessage(::base::LogLevel::kInfo, __VA_ARGS__)
#define LOG_WARNING(...) \
  ::base::PrintLogMessage(::base::LogLevel::kWarning, __VA_ARGS__)
#define LOG_ERROR(...) \
  ::base::PrintLogMessage(::base::LogLevel::kError, __VA_ARGS__)

#define TK_DCHECK(expression)                                     \
  do {                                                            \
    if (!(expression)) {                                          \
      ::base::PrintLogMessage(                                    \
          ::base::LogLevel::kError,                               \
          __FUNCTION__ "() -> assertion failed at " #expression); \
      if (TK_DBG) {                                               \
        __debugbreak();                                           \
      }                                                           \
    }                                                             \
                                                                  \
  } while (0)

#define TK_IMPOSSIBLE \
  ::base::PrintLogMessage<>(::base::LogLevel::kError, __FUNCTION__ " impossible " \
                                                                 "reached")
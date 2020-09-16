// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <fmt/format.h>

namespace noda::utils
{
  enum class LogLevel {
	Trace,
	Info,
	Warning,
	Error
  };

  // thread safe!
  void PrintLogMessage(LogLevel, const char*, const fmt::format_args&);

  template <typename... Args>
  inline void PrintLogMessage(LogLevel lvl, const Args &... args)
  {
	PrintLogMessage(lvl, fmt::make_format_args(args...));
  }

#define LOG_TRACE(...) \
  ::noda::utils::PrintLogMessage(::noda::utils::LogLevel::Trace, __VA_ARGS__)
#define LOG_INFO(...) \
  ::noda::utils::PrintLogMessage(::noda::utils::LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) \
  ::noda::utils::PrintLogMessage(::noda::utils::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) \
  ::noda::utils::PrintLogMessage(::noda::utils::LogLevel::Error, __VA_ARGS__)
} // namespace noda::utils
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdarg>
#include <base/logging.h>

#include <fmt/printf.h>
#include <base/strings/xstring.h>

namespace base {

namespace {
const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                     "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

void DefaultLogHandler(void* user_pointer, LogLevel ll, const char* msg) {
  // TODO: log to debug out.
}

constinit struct {
  void* user_pointer;
  LogHandler callback;
} log_data{nullptr, DefaultLogHandler};
}  // namespace

const char* LevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

// Note: This is the only function that may not assert.
void SetLogHandler(LogHandler callback, void* user_pointer) {
  log_data = {user_pointer, callback};
}

namespace detail {
void WriteLogMessage(LogLevel ll, const char* text, const fmt::format_args& args) {
  // optimization to get rid of the std::string construction
  fmt::basic_memory_buffer<char> buffer;
  fmt::detail::vformat_to(buffer, fmt::v8::string_view(text), args);
  buffer.push_back(0);  // will not allocate
  log_data.callback(log_data.user_pointer, ll, buffer.data());
}

void WriteLogMessagef(LogLevel ll,
                      const char* text,
                      const fmt::basic_format_args<fmt::printf_context>& args) {
  fmt::basic_memory_buffer<char> buffer;
  fmt::detail::vprintf(buffer, fmt::v8::string_view(text), args);
  buffer.push_back(0);  // will not allocate

  log_data.callback(log_data.user_pointer, ll, buffer.data());
}

void WriteLogMessage(LogLevel ll, const char* text) {
  log_data.callback(log_data.user_pointer, ll, text);
}
}  // namespace detail
}  // namespace base
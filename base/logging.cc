// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <cstdio>

namespace base {

namespace {
const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                     "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

void DefaultLogHandler(void* /*user_pointer*/,
                       const char* channel_name,
                       LogLevel ll,
                       const char* msg) {
  fprintf(stderr, "[%s] %s: %s\n", channel_name ? channel_name : "?",
          LogLevelToName(ll), msg ? msg : "");
}

constinit struct {
  void* user_pointer;
  LogHandler callback;
} log_data{nullptr, DefaultLogHandler};
}  // namespace

const char* LogLevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

void SetLogHandler(LogHandler callback, void* user_pointer) noexcept {
  if (!callback)
    callback = DefaultLogHandler;
  log_data = {user_pointer, callback};
}

void SetLogInstance(void* user_pointer) {
  log_data.user_pointer = user_pointer;
}

namespace detail {
void WriteLogMessage(const char* channel_name, LogLevel ll, const char* text) {
  log_data.callback(log_data.user_pointer, channel_name, ll, text);
}
}  // namespace detail
}  // namespace base

// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdarg>
#include "xstring.h"
#include "logging.h"

namespace base {

namespace {
const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                     "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

void DefaultLogHandler(LogLevel ll, const char* msg) {
  // TODO: log to debug out.
}

// Optimization to generate better code on log message invoking
constinit LogHandler s_callback{DefaultLogHandler};
}  // namespace

const char* LevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

void PrintLogMessagePF(LogLevel ll, const char* format...) {
  va_list ap;
  va_start(ap, format);

  // todo: safe stuff
  char buf[1024]{};
  vsprintf(buf, format, ap);
  va_end(ap);

  if (s_callback)
    s_callback(ll, buf);
}


// Note: This is the only function that may not assert.
void SetLogHandler(LogHandler callback) {
  s_callback = callback;
}

namespace detail {
void WriteLogMessage(LogLevel ll, const char* text, const fmt::format_args& args) {
  // optimization to get rid of the std::string construction
  auto buffer = fmt::memory_buffer();
  fmt::detail::vformat_to(buffer, fmt::v8::string_view(text), args);
  s_callback(ll, buffer.data());
}

void WriteLogMessage(LogLevel ll, const char* text) {
  s_callback(ll, text);
}
}  // namespace detail
}  // namespace base
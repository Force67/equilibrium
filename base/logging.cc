// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdarg>
#include "xstring.h"
#include "logging.h"

namespace base {

namespace {
constexpr char kLegalText[] = R"(

===============================================
::
:: RETK for windows, Copyright (C) 2021 Force67
:: >Universe: Dev
:: >Buildtag: <Unknown>
::
===============================================
)";

const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                     "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

LogHandler s_callback{nullptr};
}  // namespace

const char* LevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

void PrintLogMessagePF(LogLevel ll, const char* format...) {
  va_list ap;
  va_start(ap, format);

  char buf[1024]{};
  vsprintf(buf, format, ap);
  va_end(ap);

  if (s_callback)
    s_callback(ll, buf);
}

void PrintLegals() {
  LOG_INFO(kLegalText);
}

// Note: This is the only function that may not assert.
void SetLogHandler(LogHandler callback) {
  s_callback = callback;
}

namespace detail {
void WriteLogMessage(LogLevel ll,
                          const char* text,
                          const fmt::format_args& args) {
  auto fmt = fmt::vformat(text, args);

  if (s_callback)
    s_callback(ll, fmt.c_str());
}

void WriteLogMessage(LogLevel ll, const char* text) {
  if (s_callback)
    s_callback(ll, text);
}
}  // namespace detail
}  // namespace base
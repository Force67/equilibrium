// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "logging.h"

namespace base {

static constexpr char kLegalText[] = R"(

===============================================
::
:: RETK for windows, Copyright (C) 2021 Force67
:: >Universe: Dev
:: >Buildtag: <Unknown>
::
===============================================
)";

static const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                            "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

const char* LevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

static LogHandler s_callback{nullptr};

void Core_PrintLogMessage(LogLevel ll,
                          const char* text,
                          const fmt::format_args& args) {
  auto fmt = fmt::vformat(text, args);

  if (s_callback)
    s_callback(ll, fmt.c_str());
}

void PrintLegals() {
  LOG_INFO(kLegalText);
}

void InitLogging(LogHandler callback, bool surpress_logo) {
  s_callback = callback;

  if (!surpress_logo)
    PrintLegals();
}
}  // namespace base
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Logger.h"
#include "Pch.h"

namespace noda {
static const char* GetLevelName(LogLevel log_level) {
#define LVL(x)      \
  case LogLevel::x: \
    return #x
  switch (log_level) {
    LVL(Trace);
    LVL(Info);
    LVL(Warning);
    LVL(Error);
    default:
      return "???";
  }
#undef LVL
}

// pretty print me, please :-)
void PrintLogMessageImpl(LogLevel ll,
                         const char* text,
                         const fmt::format_args& args) {
  // ensure we don't print into nirvana
  if (!callui(ui_is_msg_inited).cnd)
    return;

  if (ll == LogLevel::Error) {
    callui(ui_beep, 0);
  }

  const char* levelName = GetLevelName(ll);

  auto fmt =
      fmt::format("[Noda] <{}>: {}\n", levelName, fmt::vformat(text, args));
  callui(ui_msg, fmt.c_str());
}
}  // namespace noda
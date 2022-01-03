// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>

static void PrintLogMessageImpl(base::LogLevel level, const char* text) {
  // ensure we don't print into nirvana
  if (!callui(ui_is_msg_inited).cnd)
    return;

  if (level == base::LogLevel::kError) {
    callui(ui_beep, 0);
  }

  const char* levelName = base::LevelToName(level);

  auto fmt = fmt::format("[ReTK] <{}>: {}\n", levelName, text);
  callui(ui_msg, fmt.c_str());
}

void InitIdaLogHandler() {
  base::InitLogging(PrintLogMessageImpl);
}
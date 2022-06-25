// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <main/log_handler.h>

#include <base/check.h>
#include <base/text/code_convert.h>

#if defined(OS_WIN)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringW(
    const wchar_t* lpOutputString);
#endif

namespace main {

namespace {
void LogCallback(void* user_pointer, base::LogLevel log_level, const char* message) {
  LogHandler* log_handler = nullptr;
  BUGCHECK(log_handler = reinterpret_cast<LogHandler*>(user_pointer));

  log_handler->BuildMessage(log_level, message);
}
}  // namespace

LogHandler::LogHandler() {
  base::SetLogHandler(LogCallback, this);
}

LogHandler::~LogHandler() {
  // this will reset to the default handler..
  base::SetLogHandler(nullptr, nullptr);
}

void LogHandler::BuildMessage(base::LogLevel level, const char* source_message) {
  // build a message
  auto buffer = fmt::format("[{}]: {}\n", base::LogLevelToName(level), source_message);

#if defined(OS_WIN) //|| defined(CONFIG_DEBUG)
  {
    // const char* src, size_t src_len, base::StringW* output
    base::StringW wide_text;
    BUGCHECK(base::UTF8ToWide(buffer.c_str(), buffer.length(), &wide_text));
    ::OutputDebugStringW(wide_text.c_str());
  }
#endif
}
}  // namespace main
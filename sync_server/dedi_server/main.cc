// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#if defined(OS_WIN)
#include <Windows.h>
#endif

#include <Server.h>

#include <chrono>
#include <thread>

#include <fmt/color.h>

using namespace std::chrono_literals;
using namespace sync_server;

static const char* LogLevelToString(LogLevel ll) {
  switch (ll) {
    case LogLevel::kTrace:
      return "Trace";
    case LogLevel::kInfo:
      return "Info";
    case LogLevel::kWarning:
      return "Warning";
    case LogLevel::kError:
      return "Error";
    default:
      return "???";
  }
}

int main(int argc, char** argv) {
#if defined(OS_WIN)
  SetConsoleTitleW(L"ReTK server");
#endif

  fmt::print("<<<- DedicatedMain Init ->>>\n");

  // pretty print the log to terminal
  Server::SetLogCallback([](LogLevel ll, const char* text) {
    const char* const name = LogLevelToString(ll);

#if defined(OS_WIN)
    fmt::print("[{}]: {}\n", name, text);
#else
    fmt::text_style fg_color;
    switch (ll) {
      case LogLevel::kTrace:
        fg_color = fg(fmt::color::coral) | fmt::emphasis::underline;
        break;
      case LogLevel::kInfo:
        fg_color = fg(fmt::color::white);
        break;
      case LogLevel::kWarning:
        fg_color = fg(fmt::color::yellow) | fmt::emphasis::italic;
        break;
      case LogLevel::kError:
        fg_color = fg(fmt::color::crimson) | fmt::emphasis::bold;
        break;
    }

    fmt::print(fg_color, "[{}]: {}\n", name, text);
#endif
  });

  Server server;
  const auto result = server.Initialize(true);
  if (result != Server::ResultStatus::kSuccess) {
#if defined(OS_WIN)
    fmt::print(
        "[Error] DedicatedMain() -> Failed to initialize server (errc: {})\n",
        static_cast<int>(result));
#else
    fmt::print(
        fg(fmt::color::crimson) | fmt::emphasis::bold,
        "[Error] DedicatedMain() -> Failed to initialize server (errc: {})\n",
        static_cast<int>(result));
#endif
    return 0;
  }

  while (server.IsListening()) {
    server.Update();

    std::this_thread::sleep_for(1ms);
  }

  return 0;
}
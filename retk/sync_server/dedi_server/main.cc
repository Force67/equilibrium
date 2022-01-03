// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#if defined(OS_WIN)
#include <Windows.h>
#endif

#include <Server.h>
#include <base/logging.h>

#include <chrono>
#include <thread>

#include <fmt/color.h>

using namespace std::chrono_literals;
using namespace sync_server;

int main(int argc, char** argv) {
#if defined(OS_WIN)
  SetConsoleTitleW(L"ReTK server");
#endif

  fmt::print("<<<- DedicatedMain Init ->>>\n");

  // pretty print the log to terminal
  Server::SetLogCallback([](base::LogLevel logLevel, const char* text) {
    const char* const name = base::LevelToName(logLevel);

#if defined(OS_WIN)
    fmt::print("[{}]: {}\n", name, text);
#else
    fmt::text_style fg_color;
    switch (ll) {
      case base::LogLevel::kTrace:
        fg_color = fg(fmt::color::coral) | fmt::emphasis::underline;
        break;
      case base::LogLevel::kInfo:
        fg_color = fg(fmt::color::white);
        break;
      case base::LogLevel::kWarning:
        fg_color = fg(fmt::color::yellow) | fmt::emphasis::italic;
        break;
      case base::LogLevel::kError:
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
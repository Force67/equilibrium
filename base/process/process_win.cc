// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/filesystem/path.h>
#include <base/text/code_convert.h>

// https://source.chromium.org/chromium/chromium/src/+/main:base/process/launch_win.cc?q=CreateProcess&ss=chromium&start=31
namespace base {

bool SpawnProcess(const Path& path_to_executable,
                  const base::StringRefU8 command_line) {
  BUGCHECK(command_line.IsNullTerminated(), "Invalid command_line");

  auto wide_command_line = base::UTF8ToWide(command_line);

  STARTUPINFOW startup_info{.cb = sizeof(STARTUPINFOW)};
  PROCESS_INFORMATION process_info{};

  return ::CreateProcessW(path_to_executable.c_str(), wide_command_line.data(),
                          nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup_info,
                          &process_info) == TRUE;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/path.h>
#include <base/process/process.h>
#include <base/environment_variables.h>

#if defined(OS_WIN)
#include <Windows.h>
#endif

#include <base/profiling/profiler_communication.h>

#undef GetEnvironmentVariable

namespace base {

bool AttachProfiler() {
#if defined(ENABLE_PROFILE)
  base::StringU8 path;
  if (!base::GetEnvironmentVariable(u8"EQ_TRACY_PATH", path)) {
    LOG_ERROR("Failed to find tracy executable. (Set EQ_TRACY_PATH)");
    return false;
  }

  // dispatch tracy
  if (!base::SpawnProcess(base::StringRefU8(path.c_str(), path.length()),
                          u8" -a 127.0.0.1")) {
    LOG_ERROR("Failed to start profiler");
    return false;
  }

  LOG_INFO("Launching profiler at {}", "127.0.0.1");

#if defined(OS_WIN)
  while (!::FindWindowW(L"GLFW30", L"Tracy Profiler 0.8.2")) {
    ::Sleep(10);
  }
#endif
#endif

  return true;
}
}  // namespace base
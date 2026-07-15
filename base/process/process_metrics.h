// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

#if defined(OS_WIN)
using ProcessHandle = void*;
#elif defined(OS_MAC) || defined(OS_MACOS)
using ProcessHandle = u32;  // mach_port_t
#else
using ProcessHandle = i32;  // pid_t
#endif

struct ProcessMemoryUsage {
  mem_size resident_set_bytes = 0;
  mem_size peak_resident_set_bytes = 0;
};

// Returns a borrowed native handle for the calling process. Callers must not
// close it. External handles remain owned by their creator.
BASE_EXPORT ProcessHandle GetCurrentProcessHandle();

// Queries physical memory resident for |process|. On POSIX the handle is a pid,
// on macOS a Mach task port, and on Windows a process HANDLE with query rights.
// The peak may be zero when the platform cannot provide it. Clears |usage| on
// failure.
BASE_EXPORT bool QueryProcessMemoryUsage(ProcessHandle process,
                                         ProcessMemoryUsage& usage);

}  // namespace base

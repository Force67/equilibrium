// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <windows.h>
#include <psapi.h>

namespace base {

ProcessHandle GetCurrentProcessHandle() {
  static_assert(sizeof(ProcessHandle) == sizeof(HANDLE));
  return GetCurrentProcess();
}

bool QueryProcessMemoryUsage(ProcessHandle process, ProcessMemoryUsage& usage) {
  usage = {};
  // GetCurrentProcess() is the valid pseudo-handle (HANDLE)-1, the same bit
  // pattern as INVALID_HANDLE_VALUE, and GetProcessMemoryInfo accepts it.
  if (!process)
    return false;

  PROCESS_MEMORY_COUNTERS counters{};
  counters.cb = static_cast<DWORD>(sizeof(counters));
  if (!GetProcessMemoryInfo(process, &counters, static_cast<DWORD>(sizeof(counters)))) {
    return false;
  }
  usage.resident_set_bytes = static_cast<mem_size>(counters.WorkingSetSize);
  usage.peak_resident_set_bytes = static_cast<mem_size>(counters.PeakWorkingSetSize);
  return true;
}

}  // namespace base

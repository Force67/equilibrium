// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <Windows.h>
#include <Psapi.h>

namespace base {

bool QueryCurrentProcessMemoryUsage(ProcessMemoryUsage* usage) {
  if (!usage)
    return false;
  *usage = {};

  PROCESS_MEMORY_COUNTERS counters{};
  counters.cb = static_cast<DWORD>(sizeof(counters));
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters,
                            static_cast<DWORD>(sizeof(counters)))) {
    return false;
  }
  usage->resident_set_bytes = static_cast<std::size_t>(counters.WorkingSetSize);
  usage->peak_resident_set_bytes = static_cast<std::size_t>(counters.PeakWorkingSetSize);
  return true;
}

}  // namespace base

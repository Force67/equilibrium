// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <unistd.h>

#include <stdio.h>

namespace base {

ProcessHandle GetCurrentProcessHandle() {
  return static_cast<ProcessHandle>(getpid());
}

bool QueryProcessMemoryUsage(ProcessHandle process, ProcessMemoryUsage& usage) {
  usage = {};
  if (process <= 0)
    return false;

  char path[64];
  ::snprintf(path, sizeof(path), "/proc/%d/status", process);
  ::FILE* status = ::fopen(path, "r");
  if (!status)
    return false;

  bool found_resident = false;
  char line[256];
  while (::fgets(line, sizeof(line), status)) {
    unsigned long long kib = 0;
    if (::sscanf(line, "VmRSS: %llu kB", &kib) == 1) {
      usage.resident_set_bytes = static_cast<mem_size>(kib) * 1024u;
      found_resident = true;
    } else if (::sscanf(line, "VmHWM: %llu kB", &kib) == 1) {
      usage.peak_resident_set_bytes = static_cast<mem_size>(kib) * 1024u;
    }
  }
  ::fclose(status);
  if (found_resident)
    return true;
  usage = {};
  return false;
}

}  // namespace base

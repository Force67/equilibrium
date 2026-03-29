// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "debugging.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace base {

bool IsDebuggerAttached() {
  // Read /proc/self/status and look for "TracerPid:\t<nonzero>".
  FILE* f = std::fopen("/proc/self/status", "r");
  if (!f) return false;

  char line[256];
  while (std::fgets(line, sizeof(line), f)) {
    if (std::strncmp(line, "TracerPid:\t", 11) == 0) {
      std::fclose(f);
      // TracerPid is 0 when no debugger is attached.
      return std::atoi(line + 11) != 0;
    }
  }
  std::fclose(f);
  return false;
}

}  // namespace base

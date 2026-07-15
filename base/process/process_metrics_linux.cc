// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <sys/resource.h>
#include <unistd.h>

#include <cstdio>

namespace base {

bool QueryCurrentProcessMemoryUsage(ProcessMemoryUsage* usage) {
  if (!usage)
    return false;
  *usage = {};

  unsigned long resident_pages = 0;
  std::FILE* statm = std::fopen("/proc/self/statm", "r");
  if (!statm)
    return false;
  const int fields = std::fscanf(statm, "%*s %lu", &resident_pages);
  std::fclose(statm);
  if (fields != 1)
    return false;

  const long page_size = sysconf(_SC_PAGESIZE);
  if (page_size <= 0)
    return false;
  usage->resident_set_bytes =
      static_cast<std::size_t>(resident_pages) * static_cast<std::size_t>(page_size);

  rusage resources{};
  if (getrusage(RUSAGE_SELF, &resources) == 0 && resources.ru_maxrss > 0) {
    // Linux and Android report ru_maxrss in KiB.
    usage->peak_resident_set_bytes =
        static_cast<std::size_t>(resources.ru_maxrss) * 1024u;
  }
  return true;
}

}  // namespace base

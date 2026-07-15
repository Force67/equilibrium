// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <mach/mach.h>
#include <sys/resource.h>

namespace base {

bool QueryCurrentProcessMemoryUsage(ProcessMemoryUsage* usage) {
  if (!usage)
    return false;
  *usage = {};

  mach_task_basic_info_data_t task_memory{};
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  const kern_return_t result =
      task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                reinterpret_cast<task_info_t>(&task_memory), &count);
  if (result != KERN_SUCCESS)
    return false;
  usage->resident_set_bytes = static_cast<std::size_t>(task_memory.resident_size);

  rusage resources{};
  if (getrusage(RUSAGE_SELF, &resources) == 0 && resources.ru_maxrss > 0) {
    // macOS reports ru_maxrss in bytes.
    usage->peak_resident_set_bytes = static_cast<std::size_t>(resources.ru_maxrss);
  }
  return true;
}

}  // namespace base

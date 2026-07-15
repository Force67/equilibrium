// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <mach/mach.h>
namespace base {

ProcessHandle GetCurrentProcessHandle() {
  static_assert(sizeof(ProcessHandle) == sizeof(mach_port_t));
  return static_cast<ProcessHandle>(mach_task_self());
}

bool QueryProcessMemoryUsage(ProcessHandle process, ProcessMemoryUsage& usage) {
  usage = {};
  const mach_port_t task = static_cast<mach_port_t>(process);
  if (!MACH_PORT_VALID(task))
    return false;

  mach_task_basic_info_data_t task_memory{};
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  const kern_return_t result = task_info(
      task, MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&task_memory), &count);
  if (result != KERN_SUCCESS)
    return false;
  usage.resident_set_bytes = static_cast<mem_size>(task_memory.resident_size);
  usage.peak_resident_set_bytes = static_cast<mem_size>(task_memory.resident_size_max);
  return true;
}

}  // namespace base

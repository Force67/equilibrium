// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstddef>

#include <base/export.h>

namespace base {

struct ProcessMemoryUsage {
  std::size_t resident_set_bytes = 0;
  std::size_t peak_resident_set_bytes = 0;
};

// Queries physical memory currently resident for this process. The peak may be
// zero when the platform cannot provide it. Clears |usage| on failure.
BASE_EXPORT bool QueryCurrentProcessMemoryUsage(ProcessMemoryUsage* usage);

}  // namespace base

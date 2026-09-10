// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>

namespace base {
// Spawn a profiler instance and attach to the calling process.
// Blocks the calling thread until the process is launched.
BASE_EXPORT bool AttachProfiler();
}  // namespace base
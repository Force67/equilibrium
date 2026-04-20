// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>

namespace base {
// spawn a profiler instance and attach to the calling process.
// warning: this will block the thread that its called from till the process is
// launched

// profiler_connection.cc?
BASE_EXPORT bool AttachProfiler();
}  // namespace base
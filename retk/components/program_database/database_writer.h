// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Header
// SectionHeader

#include <base/arch.h>
#include <base/containers/span.h>

namespace base {
class Path;
}

namespace program_database {

void WriteInitialDiskFile(const base::Span<byte> program,
                          const base::Path& out_path,
                          const u32 retk_version,
                          const u32 user_id);
}
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Header
// SectionHeader

#include <base/containers/span.h>

namespace base {
class Path;
}

namespace program_database {

void CreateDatabase(const base::Span<byte> program, const base::Path& out_path);
}
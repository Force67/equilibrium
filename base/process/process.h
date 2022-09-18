// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once
//https://source.chromium.org/chromium/chromium/src/+/main:base/process/process.h

#include <base/strings/string_ref.h>

namespace base {

class Path;

bool SpawnProcess(const Path& path_to_executable,
                  const base::StringRefU8 command_line);
}  // namespace base
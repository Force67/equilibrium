// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <filesystem>

namespace base {
namespace fs = std::filesystem;

bool CreateNewTempDirectory(const fs::path::string_type& prefix,
                            fs::path* new_temp_path);
}
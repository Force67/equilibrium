// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

// This file contains utility functions for dealing with the local
// filesystem.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>
#include <set>
#include <string>

#include "base/export.h"
#include "base/filesystem/file.h"
#include "base/filesystem/path.h"
#include "base/filesystem/scoped_file.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/minwin.h"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <sys/stat.h>
#include <unistd.h>
#include <base/filesystem/posix/file_descriptor_posix.h>
#include <base/filesystem/posix/eintr_wrapper.h>
#endif

#include <base/arch.h>

namespace base {

// Create a directory within another directory.
// Extra characters will be appended to |prefix| to ensure that the
// new directory does not have the same name as an existing directory.
BASE_EXPORT bool CreateTemporaryDirInDir(const Path& base_dir,
                                         const Path::BufferType& prefix,
                                         Path* new_dir);

// Backward-compatible convenience method for the above.
BASE_EXPORT bool CreateDirectory(const Path& full_path);

BASE_EXPORT bool DirectoryExists(const Path& path);

[[nodiscard]] BASE_EXPORT bool PathExists(const Path& path);

BASE_EXPORT bool CreateNewTempDirectory(const Path::BufferType& prefix,
                                        Path* new_temp_path);

BASE_EXPORT std::unique_ptr<u8[]> LoadFile(const Path& path, i64* opt_size);
}  // namespace base

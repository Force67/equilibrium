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
#include "base/io/file/file.h"
#include "base/io/file/file_path.h"
#include "base/io/file/scoped_file.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/minwin.h"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <sys/stat.h>
#include <unistd.h>
#include "base/file_descriptor_posix.h"
#include "base/posix/eintr_wrapper.h"
#endif

namespace base {

// Create a directory within another directory.
// Extra characters will be appended to |prefix| to ensure that the
// new directory does not have the same name as an existing directory.
BASE_EXPORT bool CreateTemporaryDirInDir(const FilePath& base_dir,
                                         const FilePath::BufferType& prefix,
                                         FilePath* new_dir);

// Backward-compatible convenience method for the above.
BASE_EXPORT bool CreateDirectory(const FilePath& full_path);

BASE_EXPORT bool DirectoryExists(const FilePath& path);

BASE_EXPORT bool CreateNewTempDirectory(const FilePath::BufferType& prefix,
                                        FilePath* new_temp_path);
}  // namespace base

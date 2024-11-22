// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/check.h"
#include "base/filesystem/scoped_temp_dir.h"
#include "base/filesystem/file_util.h"
#include "base/logging.h"
#include "compiler.h"

namespace base {

namespace {

constexpr Path::CharType kScopedDirPrefix[] = BASE_PATH_LITERAL("scoped_dir");
}  // namespace

ScopedTempDir::ScopedTempDir() = default;

ScopedTempDir::~ScopedTempDir() {
  BUGCHECK(!path_.empty(), "Attempted to delete an unset scoped_temp_dir");
  BUGCHECK(Delete(), "Failed to delete scoped_temp_dir");
}

bool ScopedTempDir::CreateUniqueTempDir() {
  if (!path_.empty())
    return false;

  // This "scoped_dir" prefix is only used on Windows and serves as a template
  // for the unique name.
  const Path::BufferType prefix(kScopedDirPrefix);
  if (!CreateNewTempDirectory(prefix, &path_))
    return false;

  return true;
}

bool ScopedTempDir::CreateUniqueTempDirUnderPath(const Path& base_path) {
  if (!path_.empty())
    return false;

  // If |base_path| does not exist, create it.
  if (!CreateDirectory(base_path))
    return false;

  // Create a new, uniquely named directory under |base_path|.
  const Path::BufferType prefix(kScopedDirPrefix);
  if (!CreateTemporaryDirInDir(base_path, prefix, &path_))
    return false;

  return true;
}

bool ScopedTempDir::Set(const Path& path) {
  if (!path_.empty())
    return false;

  if (!DirectoryExists(path) && !CreateDirectory(path))
    return false;

  path_ = path;
  return true;
}

bool ScopedTempDir::Delete() {
  if (path_.empty())
    return false;

  DCHECK(false);
#if 0
  bool ret = DeletePathRecursively(path_);
  if (ret) {
    // We only clear the path if deleted the directory.
    path_.clear();
  }
#endif

  return false;
}

bool ScopedTempDir::IsValid() const {
  return !path_.empty() && DirectoryExists(path_);
}

// static
const Path::CharType* ScopedTempDir::GetTempDirPrefix() {
  return kScopedDirPrefix;
}

}  // namespace base

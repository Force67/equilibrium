// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/io/file/scoped_temp_dir.h"
#include "base/check.h"

#include "base/io/file/file_util.h"
#include "base/logging.h"

namespace base {

namespace {

constexpr FilePath::CharType kScopedDirPrefix[] = L"scoped_dir";

}  // namespace

ScopedTempDir::ScopedTempDir() = default;

ScopedTempDir::~ScopedTempDir() {
  if (!path_.empty() && !Delete()) {
    __debugbreak();
  }
}

bool ScopedTempDir::CreateUniqueTempDir() {
  if (!path_.empty())
    return false;

  // This "scoped_dir" prefix is only used on Windows and serves as a template
  // for the unique name.
  if (!CreateNewTempDirectory(kScopedDirPrefix, &path_))
    return false;

  return true;
}

bool ScopedTempDir::CreateUniqueTempDirUnderPath(const FilePath& base_path) {
  if (!path_.empty())
    return false;

  // If |base_path| does not exist, create it.
  if (!CreateDirectory(base_path))
    return false;

  // Create a new, uniquely named directory under |base_path|.
  if (!CreateTemporaryDirInDir(base_path, kScopedDirPrefix, &path_))
    return false;

  return true;
}

bool ScopedTempDir::Set(const FilePath& path) {
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

  __debugbreak();

  return false;
}

bool ScopedTempDir::IsValid() const {
  return !path_.empty() && DirectoryExists(path_);
}

// static
const FilePath::CharType* ScopedTempDir::GetTempDirPrefix() {
  return kScopedDirPrefix;
}

}  // namespace base

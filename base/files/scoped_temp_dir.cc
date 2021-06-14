// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "scoped_temp_dir.h"
#include <logging.h>

namespace base {

constexpr fs::path::value_type kScopedDirPrefix[] = L"scoped_dir";

ScopedTempDir::ScopedTempDir() = default;
ScopedTempDir::~ScopedTempDir() {
  if (!path_.empty() && !Delete())
    LOG_TRACE("Could not delete temp dir in dtor.");
}

bool ScopedTempDir::CreateUniqueTempDir() {
  if (!path_.empty())
    return false;

  if (!CreateNewTempDirectory(kScopedDirPrefix, &path_))
    return false;

  return true;
}

bool ScopedTempDir::Delete() {
  if (path_.empty())
    return false;

  bool ret = fs::remove_all(path_);
  if (ret) {
    // We only clear the path if deleted the directory.
    path_.clear();
  }

  return ret;
}

fs::path& ScopedTempDir::Path() {
  TK_DCHECK(!path_.empty());
  return path_;
}

// static
const fs::path::value_type* ScopedTempDir::GetTempDirPrefix() {
  return kScopedDirPrefix;
}
}  // namespace base
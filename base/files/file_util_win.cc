// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <chrono>
#include <string>
#include "file_util.h"

namespace base {
namespace {

bool GetTempDir(fs::path& path) {
  wchar_t temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPathW(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
    return false;

  path = temp_path;
  return true;
}

bool CreateTemporaryDirInDir(const fs::path& base_dir,
                             const fs::path::string_type& prefix,
                             fs::path* new_dir) {
  constexpr int kAttempts = 50;

  fs::path path_to_create;
  for (int i = 0; i < kAttempts; i++) {
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

    std::wstring newDir;
    newDir.assign(prefix);
    newDir.append(std::to_wstring(::GetCurrentProcessId()));
    newDir.push_back('_');
    newDir.append(std::to_wstring(msec));

    path_to_create = base_dir / newDir;
    if (::CreateDirectoryW(path_to_create.c_str(), nullptr)) {
      *new_dir = path_to_create;
      return true;
    }
  }

  return false;
}
}  // namespace

bool CreateNewTempDirectory(const fs::path::string_type& prefix,
                            fs::path* new_temp_path) {
  fs::path temp_dir;
  if (!GetTempDir(temp_dir))
    return false;

  return CreateTemporaryDirInDir(temp_dir, prefix, new_temp_path);
}
}  // namespace base
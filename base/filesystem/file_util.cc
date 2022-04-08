// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/check.h"
#include "base/filesystem/file_util.h"

namespace base {

// quick and dirty way of loading a file, needs to be revised later.
std::unique_ptr<u8[]> LoadFile(const Path& path, i64* opt_size) {
  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid()) {
    return nullptr;
  }

  auto length = file.Seek(base::File::Whence::FROM_END, 0);
  if (opt_size)
    *opt_size = length;

  auto content = std::make_unique<u8[]>(length);
  file.Seek(base::File::Whence::FROM_BEGIN, 0);
  size_t read = file.Read(0, reinterpret_cast<char*>(content.get()), length);
  if (read == length) {
    return content;
  }

  return nullptr;
}

}  // namespace base

// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/io/file/file_path.h"
#include "base/string/code_convert.h"

namespace base {

FilePath::FilePath(const FilePath& other) {
  path_buf_ = other.path_buf_;
}

FilePath::FilePath(const BufferType& path) {
  path_buf_ = path;
}

FilePath FilePath::DirName() const {
  return {};
}

FilePath FilePath::BaseName() const {
  return {};
}

FilePath FilePath::Extension() const {
  return {};
}

}  // namespace base

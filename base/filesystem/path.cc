// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/path.h>
#include <base/text/code_convert.h>

namespace base {

Path::Path(const Path& other) {
  path_buf_ = other.path_buf_;
}

Path::Path(const BufferType& path) {
  path_buf_ = path;
}

Path& Path::operator/=(const Path& other) {
  path_buf_ += (PATH_SEP_MACRO + other.path_buf_);
  return *this;
}

Path Path::DirName() const {
  return {};
}

Path Path::BaseName() const {
  return {};
}

Path Path::Extension() const {
  return {};
}

}  // namespace base

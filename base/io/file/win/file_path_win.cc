// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "io/file/file_path.h"
#include "string/code_convert.h"

namespace base {

FilePath::FilePath(const char* type) {
  path_buf_ = base::UTF8ToWide(type);
}

FilePath& FilePath::operator/(const char* rhs) {
  // TODO: this requires tuning
  auto native = kSeperator + base::UTF8ToWide(rhs);
  path_buf_ += native;
  return *this;
}

FilePath& FilePath::operator/(const wchar_t* rhs) {
  auto native = kSeperator + BufferType(rhs);
  path_buf_ += native;
  return *this;
}
}  // namespace base
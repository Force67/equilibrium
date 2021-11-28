// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "io/file/file_path.h"
#include "string/code_convert.h"

namespace base {

FilePath::FilePath(const char* type) {
  path_buf_ = base::UTF8ToWide(type);
}

FilePath::FilePath(const wchar_t* type) {
  path_buf_ = type;
}
}  // namespace base
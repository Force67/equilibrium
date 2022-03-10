// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "text/code_convert.h"

namespace base {

Path::Path(const char* type) {
  path_buf_ = base::UTF8ToWide(type);
}

Path::Path(const wchar_t* type) {
  path_buf_ = type;
}
}  // namespace base
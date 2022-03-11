// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "text/code_convert.h"

namespace base {
Path::Path(const char* ascii_only) {
  // this will assert if non ascii gets passed.
  path_buf_ = base::ASCIIToWide(ascii_only);
}

Path::Path(const base::StringRefU8 utf8) {
  path_buf_ = base::UTF8ToWide(utf8);
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = wide_text;
}
}  // namespace base
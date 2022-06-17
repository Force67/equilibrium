// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "text/code_convert.h"

namespace base {
Path::Path(const char* ascii_only) {
  DCHECK(base::IsASCII(ascii_only));
  path_buf_ = ascii_only;
}

Path::Path(const base::StringRefU8 utf8) {
  path_buf_ = reinterpret_cast<const char*>(utf8.c_str());
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = base::WideToUTF8(wide_text);
}
}  // namespace base
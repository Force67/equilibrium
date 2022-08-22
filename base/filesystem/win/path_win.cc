// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "text/code_convert.h"

namespace base {
Path::Path(const char* ascii_only) {
  // this will assert if non ascii gets passed.
  path_buf_ = base::ASCIIToWide(ascii_only);
  NormalizePath(path_buf_);
}

Path::Path(const base::StringRefU8 utf8) {
  path_buf_ = base::UTF8ToWide(utf8);
  NormalizePath(path_buf_);
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = wide_text.data();
  NormalizePath(path_buf_);
}

void Path::NormalizePath(BufferType& buffer) {
  for (auto i = 0; i < buffer.size(); i++) {
    auto& c = buffer[i];
    bool matches = c == L'/';

    // windows folk like to put two forward slashes in their path names, so we have
    // to check if the next character also contains a forward slash, and if so,
    // remove it
    if (matches && i + 1 <= buffer.size()) {
      if (buffer[i + 1] == L'/') {
        buffer.erase(buffer.begin() + (i + 1));
      }
    }
    if (matches) {
      c = BASE_PATH_SEP_MACRO;
    }
  }
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "text/code_convert.h"
#include <base/text/code_point_validation.h>
#include <base/text/code_convert.h>

namespace base {
Path::Path(const char* ascii_only) {
  // this will assert if non ascii gets passed.
  path_buf_ = base::ASCIIToWide(ascii_only);
  Normalize(path_buf_);
}

Path::Path(const base::StringRefU8 utf8) {
  path_buf_ = base::UTF8ToWide(utf8);
  Normalize(path_buf_);
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = wide_text.data();
  Normalize(path_buf_);
}

void Path::Normalize(BufferType& buffer) {
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

base::String Path::ToAsciiString() const {
  DCHECK(base::DoIsStringASCII(path_buf_.c_str(),
                               base::CountStringLength(path_buf_.c_str())),
         "Path must be ASCII only");
  return base::WideToASCII(path_buf_);
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"

#include "text/code_conv_validate.h"
#include "text/code_convert.h"

#include "strings/char_algorithms.h"

namespace base {
Path::Path(const char* ascii_only) {
  DCHECK(base::DoIsStringASCII(ascii_only, base::CountStringLength(ascii_only)),
         "Path must be ASCII only");
  path_buf_ = reinterpret_cast<const char8_t*>(ascii_only);
  Normalize(path_buf_);
}

Path::Path(const base::StringRefU8 utf8) {
  path_buf_ = utf8.c_str();
  Normalize(path_buf_);
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = base::WideToUTF8(wide_text);
  Normalize(path_buf_);
}

void Path::Normalize(BufferType& buffer) {
  IMPOSSIBLE;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"

#include "text/code_point_validation.h"
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
  path_buf_ = Path::BufferType(utf8.begin(), utf8.length());
  Normalize(path_buf_);
}

Path::Path(const base::StringRefW wide_text) {
  path_buf_ = base::WideToUTF8(wide_text);
  Normalize(path_buf_);
}

bool Path::AppendExtension(const char* ascii_only, const bool ensure_dot) {
  DCHECK(base::DoIsStringASCII(ascii_only, base::CountStringLength(ascii_only)),
         "Extension must be ASCII only");
  if (ascii_only[0] != '.' && ensure_dot) {
    path_buf_ += '.';
  }
  path_buf_ += base::StringU8((const char8_t*)ascii_only);
}

void Path::Normalize(BufferType& buffer) {
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    auto& c = buffer[i];
    bool matches = (c == u8'/');

    if (matches && i + 1 < buffer.size()) {  // Corrected the index bound check
      if (buffer[i + 1] == u8'/') {
        buffer.erase(buffer.begin() + i + 1);
        --i;  // Decrement i to re-check this index in next loop iteration
      }
    }

    if (matches) {
      c = BASE_PATH_SEP_MACRO;
    }
  }
}
}  // namespace base

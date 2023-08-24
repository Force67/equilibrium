// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#include "filesystem/path.h

namespace base {
// Life isn't easy on Linux.  We have to deal with the fact that many apis take
// only a ascii char as parameter. This is a horrible hack, defeating the point
// of storing stuff in utf8 internally, but for now it's the only way.
base::String Path::ToAsciiString() const {
  DCHECK(base::DoIsStringASCII(path_buf_.c_str(),
                               base::CountStringLength(path_buf_.c_str())),
         "Path must be ASCII only");

  return base::String(reinterpret_cast<const char*>(path_buf_.c_str()));
}
}
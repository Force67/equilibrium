// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/xstring.h>

namespace base {

class BASE_EXPORT Path {
 public:
#if defined(OS_WIN)
  static constexpr wchar_t kSeperator = L'\\';
  static constexpr char kSeperatorO = '\\';
#define PATH_SEP_MACRO L'\\'
  using CharType = wchar_t;
#else
  static constexpr char kSeperator = '/';
  using CharType = char;
#define PATH_SEP_MACRO '/'
#endif

  using BufferType = base::BasicString<CharType>;

  Path() = default;
  // from raw c string
  Path(const char* path);
  Path(const wchar_t* path);
  // from x string
  Path(const BufferType& path);
  // from other.
  Path(const Path& other);

  // this surely can be improved
  [[nodiscard]] friend Path operator/(const Path& lhs, const Path& rhs) {
    return Path(lhs.path_buf_ + PATH_SEP_MACRO + rhs.path_buf_);
  }

  // result may be ignored
  Path& operator/=(const Path& other);

  // is the path valid
  operator bool() { return !empty(); }

  [[nodiscard]] Path DirName() const;
  [[nodiscard]] Path BaseName() const;
  [[nodiscard]] Path Extension() const;

  // cxx adapters
  inline bool empty() const { return path_buf_.empty(); }
  const CharType* c_str() const { return path_buf_.c_str(); }

 private:
  BufferType path_buf_;
};
}  // namespace base
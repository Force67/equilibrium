// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "base/export.h"
#include "base/xstring.h"

namespace base {

class BASE_EXPORT FilePath {
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

  using BufferType = base::XString<CharType>;

  FilePath() = default;
  // from raw c string
  FilePath(const char* path);
  FilePath(const wchar_t* path);
  // from x string
  FilePath(const BufferType& path);
  // from other.
  FilePath(const FilePath& other);

  // this surely can be improved
  [[nodiscard]] friend FilePath operator/(const FilePath& lhs,
                                          const FilePath& rhs) {
    return FilePath(lhs.path_buf_ + PATH_SEP_MACRO + rhs.path_buf_);
  }

  // is the path valid
  operator bool() { return !empty(); }

  [[nodiscard]] FilePath DirName() const;
  [[nodiscard]] FilePath BaseName() const;
  [[nodiscard]] FilePath Extension() const;

  // cxx adapters
  inline bool empty() const { return path_buf_.empty(); }
  const CharType* c_str() const { return path_buf_.c_str(); }

 private:
  BufferType path_buf_;
};

using Path = FilePath;
}  // namespace base

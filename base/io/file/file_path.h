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
  explicit FilePath(const char* type);

  FilePath& operator/(const char* rhs);
  FilePath& operator/(const wchar_t* rhs);

  [[nodiscard]] FilePath BaseName() const;
  [[nodiscard]] FilePath Extension() const;

  const CharType* c_str() const { return path_buf_.c_str(); }
 private:
  BufferType path_buf_;
};
}  // namespace base
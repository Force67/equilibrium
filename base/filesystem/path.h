// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/strings/string_ref.h>

namespace base {

#if defined(OS_WIN)
#define PATH_SEP_MACRO L'\\'
static constexpr wchar_t kSeperator = L'\\';
#else
#define PATH_SEP_MACRO '/'
static constexpr char kSeperator = '/';
#endif

class BASE_EXPORT Path {
 public:
#if defined(OS_WIN)
  using CharType = wchar_t;
#elif (OS_POSIX)
  using CharType = char;
#endif
  using BufferType = base::BasicString<CharType>;

  Path() = default;
  Path(const char* ascii_only);
  Path(const base::StringRefU8);
  Path(const base::StringRefW);
  Path(const BufferType& path);
  // from other.
  Path(const Path& other);

  // this surely can be improved
  [[nodiscard]] friend Path operator/(const Path& lhs, const Path& rhs) {
    return Path(lhs.path_buf_ + PATH_SEP_MACRO + rhs.path_buf_);
  }

  // result may be ignored
  Path& operator/=(const Path& other);

  bool operator!=(const Path& other) {
    return path_buf_ != other.path_buf_;
  }

  bool operator==(const Path& other) {
    return path_buf_ == other.path_buf_;
  }

  // is the path valid
  operator bool() {
    return !empty();
  }

  [[nodiscard]] Path DirName() const;
  [[nodiscard]] Path BaseName() const;
  [[nodiscard]] Path Extension() const;

  // cxx adapters
  inline bool empty() const {
    return path_buf_.empty();
  }
  const CharType* c_str() const {
    return path_buf_.c_str();
  }

  inline BufferType path() const {
    return path_buf_;
  }

 private:
  BufferType path_buf_;
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/strings/string_ref.h>

namespace base {

// ugly macros
#if defined(OS_WIN)
#define BASE_PATH_SEP_MACRO L'\\'
#define BASE_PATH_LITERAL(x) L##x
#else
#define BASE_PATH_SEP_MACRO '/'
#define BASE_PATH_LITERAL(x) L##x
#endif

class BASE_EXPORT Path {
 public:
#if defined(OS_WIN)
  // On Windows, paths are encoded in 2 byte UTF-16
  using CharType = wchar_t;
  // preffered platform seperators, use when having to deal with native paths, else
  // prefer path encoding we use
  static constexpr wchar_t kSeperator = L'\\';
#elif (OS_POSIX)
  // On MacOs,these are encoded in utf8, linux encoding is not strictly specified
  using CharType = char;
  static constexpr char kSeperator = '/';
#endif
  // The special path component meaning "this directory."
  static constexpr CharType kCurrentDirectory[] = BASE_PATH_LITERAL(".");
  // The special path component meaning "the parent directory."
  static constexpr CharType kParentDirectory[] = BASE_PATH_LITERAL("..");
  // The character used to identify a file extension.
  static constexpr CharType kExtensionSeparator = BASE_PATH_LITERAL('.');

  using BufferType = base::BasicString<CharType>;

  Path() = default;

  // all of these will CHECK if the desired encoding is violated
  Path(const char* ascii_only);
  Path(const base::StringRefU8);
  Path(const base::StringRefW);
  Path(const BufferType& path);
  // from other.
  Path(const Path& other);

  // append operations
  friend Path operator/(const Path& lhs, const Path& rhs);
  Path& operator/=(const Path& other);

  operator bool() { return !empty(); }
  bool operator!=(const Path& other) const { return path_buf_ != other.path_buf_; }
  bool operator==(const Path& other) const { return path_buf_ == other.path_buf_; }

  void Append(const Path&);

  [[nodiscard]] Path DirName() const;
  [[nodiscard]] Path BaseName() const;
  [[nodiscard]] Path Extension() const;

  // cxx adapters
  inline bool empty() const { return path_buf_.empty(); }
  const CharType* c_str() const { return path_buf_.c_str(); }
  inline BufferType path() const { return path_buf_; }

 private:
  void StripTrailingSeparators();

 private:
  BufferType path_buf_;
};
}  // namespace base
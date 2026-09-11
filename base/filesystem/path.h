// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/strings/string_ref.h>

namespace base {

// Path separator macros.
#if defined(OS_WIN)
#define BASE_PATH_SEP_MACRO L'\\'
#define BASE_PATH_LITERAL(x) L##x
#endif

#if defined(OS_POSIX)
#define BASE_PATH_SEP_MACRO '/'
#define BASE_PATH_LITERAL(x) u8##x
#endif

class BASE_EXPORT Path {
 public:
#if defined(OS_WIN)
  // On Windows, paths are encoded in 2 byte UTF-16
  using CharType = wchar_t;
  // preffered platform seperators, use when having to deal with native paths,
  // else prefer path encoding we use
  static constexpr wchar_t kSeperator = L'\\';
#endif

#if defined(OS_POSIX)
  // macOS encodes filenames as UTF-8. Linux does not strictly specify an
  // encoding: GTK assumes UTF-8, Qt uses the user's locale, and neither has a
  // reliable external override. Modern distributions use UTF-8 locales and
  // translate foreign mounts, so UTF-8 is enforced here too. To be fully safe,
  // treat filenames only as "NUL-terminated, '/'-delimited bytes".
  using CharType = char8_t;
  static constexpr CharType kSeperator = u8'/';
#endif

  // The special path component meaning "this directory."
  static constexpr CharType kCurrentDirectory[] = BASE_PATH_LITERAL(".");
  // The special path component meaning "the parent directory."
  static constexpr CharType kParentDirectory[] = BASE_PATH_LITERAL("..");
  // The character used to identify a file extension.
  static constexpr CharType kExtensionSeparator = BASE_PATH_LITERAL('.');

  using BufferType = base::XBasicString<CharType>;

  Path() = default;

  // all of these will CHECK if the desired encoding is violated
  /*implicit*/ Path(const char* ascii_only);  // implicit to allow: path / blah.c_str()
  Path(const base::StringRefU8);
  Path(const base::StringRefW);
  explicit Path(const BufferType& path);
  // from other.
  /*implicit*/ Path(const Path& other);
  Path& operator=(const Path& other) = default;
  // from base::String (delegates to const char*)
  /*implicit*/ Path(const base::String& str) : Path(str.c_str()) {}

  // append operations
  friend Path operator/(const Path& lhs, const Path& rhs);
  Path& operator/=(const Path& other);

  operator bool() { return !empty(); }
  bool operator!=(const Path& other) const { return path_buf_ != other.path_buf_; }
  bool operator==(const Path& other) const { return path_buf_ == other.path_buf_; }

  // append another path to this path, and insert a seperator in between of them
  Path& Append(const Path&);

  bool AppendExtension(const char* ascii_only, const bool ensure_dot = true);

  // modifies the buffer to the internal path notation e.g. backslashes on
  // windows or forward slashes on *nix
  static void Normalize(BufferType&);

  // Returns the directory containing this path, stripping the file component.
  // A single component yields kCurrentDirectory, the root yields itself.
  // Navigation is not resolved: DirName("../a") == "..".
  [[nodiscard]] Path DirName() const;

  // Returns the last path component, file or directory. Only BaseName of the
  // root directory returns an absolute path.
  [[nodiscard]] Path BaseName() const;
  [[nodiscard]] Path Extension() const;

  // converts the path to an ascii string, if possible
  [[nodiscard]] base::String ToAsciiString() const;

  // cxx adapters
  [[nodiscard]] inline bool empty() const { return path_buf_.empty(); }

  [[nodiscard]] inline const CharType* c_str() const { return path_buf_.c_str(); }

  [[nodiscard]] inline const BufferType& path() const { return path_buf_; }

  [[nodiscard]] inline auto length() const { return path_buf_.length(); }

 private:
  void StripTrailingSeparators();

 private:
  BufferType path_buf_;
};
}  // namespace base
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
  // preffered platform seperators, use when having to deal with native paths, else
  // prefer path encoding we use
  static constexpr wchar_t kSeperator = L'\\';
#endif

#if defined(OS_POSIX)
  // On MacOs,these are encoded in utf8, linux encoding is not strictly specified
  // however we aim to enforce utf8 on linux too

  // Specifically, Glib (used by Gtk+ apps) assumes that all file names are UTF-8
  // encoded, regardless of the user's locale. This may be overridden with the
  // environment variables G_FILENAME_ENCODING and G_BROKEN_FILENAMES. On the other
  // hand, Qt defaults to assuming that all file names are encoded in the current
  // user's locale. An individual application may choose to override this assumption,
  // though I do not know of any that do, and there is no external override switch.
  // Modern Linux distributions are set up such that all users are using UTF-8
  // locales and paths on foreign filesystem mounts are translated to UTF-8, so this
  // difference in strategies generally has no effect. However, if you really want to
  // be safe, you cannot assume any structure about filenames beyond "NUL-terminated,
  // '/'-delimited sequence of bytes".
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
  Path(const char* ascii_only);
  Path(const base::StringRefU8);
  Path(const base::StringRefW);
  Path(const BufferType& path);
  // from other.
  Path(const Path& other);

  // append operations
  friend Path operator/(const Path& lhs, const Path& rhs);
  Path& operator/=(const Path& other);

  operator bool() {
    return !empty();
  }
  bool operator!=(const Path& other) const {
    return path_buf_ != other.path_buf_;
  }
  bool operator==(const Path& other) const {
    return path_buf_ == other.path_buf_;
  }

  // append another path to this path, and insert a seperator in between of them
  Path& Append(const Path&);



  // modifies the buffer to the internal path notation e.g. backslashes on windows or
  // forward slashes on *nix
  static void Normalize(BufferType&);

  // returns a Path corresponding to the directory containing the path
  // named by this object, stripping away the file component.  If this object
  // only contains one component, returns a FilePath identifying
  // kCurrentDirectory.  If this object already refers to the root directory,
  // returns a Path identifying the root directory. Please note that this
  // doesn't resolve directory navigation, e.g. the result for "../a" is "..".
  [[nodiscard]] Path DirName() const;

  // returns a Path corresponding to the last path component of this
  // object, either a file or a directory.  If this object already refers to
  // the root directory, returns a Path identifying the root directory;
  // this is the only situation in which BaseName will return an absolute path.
  [[nodiscard]] Path BaseName() const;
  [[nodiscard]] Path Extension() const;

  // converts the path to an ascii string, if possible
  [[nodiscard]] base::String ToAsciiString() const;

  // cxx adapters
  [[nodiscard]] inline bool empty() const {
    return path_buf_.empty();
  }

  [[nodiscard]] inline const CharType* c_str() const {
    return path_buf_.c_str();
  }

  [[nodiscard]] inline const BufferType& path() const {
    return path_buf_;
  }

  [[nodiscard]] inline auto length() const {
    return path_buf_.length();
  }

 private:
  void StripTrailingSeparators();

 private:
  BufferType path_buf_;
};
}  // namespace base
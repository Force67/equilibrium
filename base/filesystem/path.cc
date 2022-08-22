// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/path.h>
#include <base/text/code_convert.h>

namespace base {

namespace {
#define FILE_PATH_USES_DRIVE_LETTERS
#define FILE_PATH_USES_WIN_SEPARATORS

template <typename TChar>
mem_size FindDriveLetter(base::BasicStringRef<TChar> path) {
#if defined(FILE_PATH_USES_DRIVE_LETTERS)
  // This is dependent on an ASCII-based character set, but that's a
  // reasonable assumption.  iswalpha can be too inclusive here.
  if (path.length() >= 2 && path[1] == L':' &&
      ((path[0] >= L'A' && path[0] <= L'Z') ||
       (path[0] >= L'a' && path[0] <= L'z'))) {
    return 1;
  }
#endif  // FILE_PATH_USES_DRIVE_LETTERS
  return base::kStringNotFoundPos;
}

// this is actually an array of chars.
static constexpr Path::CharType kSeparators[] =
#if defined(FILE_PATH_USES_WIN_SEPARATORS)
    BASE_PATH_LITERAL("\\/");
#else   // FILE_PATH_USES_WIN_SEPARATORS
    BASE_PATH_LITERAL("/");
#endif  // FILE_PATH_USES_WIN_SEPARATORS

constexpr auto kSeparatorsLength = sizeof(kSeparators) / sizeof(Path::CharType);

template <typename T>
bool IsSeparator(T character) {
  for (size_t i = 0; i < kSeparatorsLength - 1; ++i) {
    if (character == kSeparators[i]) {
      return true;
    }
  }
  return false;
}
}  // namespace

Path::Path(const Path& other) : path_buf_(other.path_buf_) {
  NormalizePath(path_buf_);
}
Path::Path(const BufferType& path) : path_buf_(path) {
  NormalizePath(path_buf_);
}

void Path::Append(const Path& other) {
  path_buf_.push_back(BASE_PATH_SEP_MACRO);
  path_buf_.append(other.path_buf_.c_str(), other.path_buf_.length());
}

Path& Path::operator/=(const Path& other) {
  Append(other);
  return *this;
}

Path operator/(const Path& lhs, const Path& rhs) {
  Path path = lhs;
  path.Append(rhs);
  return path;
}

void Path::StripTrailingSeparators() {
  // If there is no drive letter, start will be 1, which will prevent stripping
  // the leading separator if there is only one separator.  If there is a drive
  // letter, start will be set appropriately to prevent stripping the first
  // separator following the drive letter, if a separator immediately follows
  // the drive letter.
  auto start = FindDriveLetter<CharType>(path_buf_) + 2;
  auto last_stripped = BufferType::npos;

  for (auto pos = path_buf_.length(); pos > start && IsSeparator(path_buf_[pos - 1]);
       --pos) {
    // If the string only has two separators and they're at the beginning,
    // don't strip them, unless the string began with more than two separators.
    if (pos != start + 1 || last_stripped == start + 2 ||
        !IsSeparator(path_buf_[start - 1])) {
      path_buf_.resize(pos - 1);
      last_stripped = pos;
    }
  }
}

Path Path::DirName() const {
  Path new_path(path_buf_);
  new_path.StripTrailingSeparators();
  // The drive letter, if any, always needs to remain in the output.  If there
  // is no drive letter, as will always be the case on platforms which do not
  // support drive letters, letter will be npos, or -1, so the comparisons and
  // resizes below using letter will still be valid.
  auto letter = FindDriveLetter<CharType>(new_path.path_buf_);

  BufferType::size_type last_separator = new_path.path_buf_.find_last_of(
      kSeparators, BufferType::npos, kSeparatorsLength - 1);
  if (last_separator == BufferType::npos) {
    // path_ is in the current directory.
    new_path.path_buf_.resize(letter + 1);
  } else if (last_separator == letter + 1) {
    // path_ is in the root directory.
    new_path.path_buf_.resize(letter + 2);
  } else if (last_separator == letter + 2 &&
             IsSeparator(new_path.path_buf_[letter + 1])) {
    // path_ is in "//" (possibly with a drive letter); leave the double
    // separator intact indicating alternate root.
    new_path.path_buf_.resize(letter + 3);
  } else if (last_separator != 0) {
    // path_ is somewhere else, trim the basename.
    new_path.path_buf_.resize(last_separator);
  }

  new_path.StripTrailingSeparators();
  if (!new_path.path_buf_.length())
    new_path.path_buf_ = kCurrentDirectory;

  return new_path;
}

Path Path::BaseName() const {
  Path new_path(path_buf_);
  new_path.StripTrailingSeparators();
  // remove drive letter
  auto letter = FindDriveLetter<CharType>(new_path.path_buf_);
  if (letter != BufferType::npos)
    new_path.path_buf_.erase(0, letter + 1);

  // keep everything after the final separator, but if the pathname is only
  // one character and it's a separator, leave it alone.
  auto last_separator = new_path.path_buf_.find_last_of(
      kSeparators, BufferType::npos, kSeparatorsLength - 1);

  if (last_separator != BufferType::npos &&
      last_separator < new_path.path_buf_.length() - 1) {
    new_path.path_buf_.erase(0, last_separator + 1);
  }

  return new_path;
}

Path Path::Extension() const {
    #if 0
  Path base(BaseName());
  const auto dot = ExtensionSeparatorPosition(base.path_buf_);
  if (dot == BufferType::npos)
    return {};

  return base.path_buf_.substr(dot, BufferType::npos);
  #endif
  return {};
}
}  // namespace base

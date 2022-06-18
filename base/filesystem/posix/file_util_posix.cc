// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <base/filesystem/file_util.h>

#include <dirent.h>
#include <fcntl.h>  // for O_NONBLOCK etc

#include <base/export.h>
#include <build/build_config.h>
#include <base/strings/string_ref.h>

#include <base/logging.h>
#include <base/compiler.h>
#include <base/containers/vector.h>
#include <base/containers/adapters.h>
#include <base/filesystem/path.h>
#include <base/threading/scoped_blocking_call.h>

namespace base {

namespace {

constexpr char kIdentifier[] = "XXXXXX";

Path FormatTemporaryFileName(const Path::CharType* identifier) {
  base::String prefix = ".org.equi.Equilibrium.";
  prefix += identifier;
  return Path(prefix);
}

base::Path GetTempTemplate() {
  return FormatTemporaryFileName(kIdentifier);
}

// Appends |mode_char| to |mode| before the optional character set encoding; see
// https://www.gnu.org/software/libc/manual/html_node/Opening-Streams.html for
// details.
base::StringRef AppendModeCharacter(base::StringRef mode, char mode_char) {
  base::String result = mode.c_str();
  size_t comma_pos = result.find(',');
  result.insert(comma_pos == base::StringRef::npos ? result.length() : comma_pos, 1,
                mode_char);
  return result;
}
}  // namespace

Path MakeAbsolutePath(const Path& input) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  char full_path[PATH_MAX];
  if (realpath(input.c_str(), full_path) == nullptr)
    return Path();
  return Path(full_path);
}

bool SetNonBlocking(int fd) {
  const int flags = fcntl(fd, F_GETFL);
  if (flags == -1)
    return false;
  if (flags & O_NONBLOCK)
    return true;
  if (HANDLE_EINTR(fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1)
    return false;
  return true;
}

bool SetCloseOnExec(int fd) {
  const int flags = fcntl(fd, F_GETFD);
  if (flags == -1)
    return false;
  if (flags & FD_CLOEXEC)
    return true;
  if (HANDLE_EINTR(fcntl(fd, F_SETFD, flags | FD_CLOEXEC)) == -1)
    return false;
  return true;
}

bool PathExists(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return access(path.c_str(), F_OK) == 0;
}

bool DirectoryExists(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  stat_wrapper_t file_info;
  if (File::Stat(path.c_str(), &file_info) != 0)
    return false;
  return S_ISDIR(file_info.st_mode);
}

bool ReadFromFD(int fd, char* buffer, size_t bytes) {
  size_t total_read = 0;
  while (total_read < bytes) {
    ssize_t bytes_read =
        HANDLE_EINTR(read(fd, buffer + total_read, bytes - total_read));
    if (bytes_read <= 0)
      break;
    total_read += bytes_read;
  }
  return total_read == bytes;
}

static bool CreateTemporaryDirInDirImpl(const Path& base_dir,
                                        const Path& name_tmpl,
                                        Path* new_dir) {
  ScopedBlockingCall scoped_blocking_call(
      FROM_HERE, BlockingType::MAY_BLOCK);  // For call to mkdtemp().
  // DCHECK(EndsWith(name_tmpl.path(), "XXXXXX"),
  //        "Directory name template must end with \"XXXXXX\".");

  Path sub_dir = base_dir / name_tmpl;
  base::String sub_dir_string = sub_dir.path();

  // this should be OK since mkdtemp just replaces characters in place
  char* buffer = const_cast<char*>(sub_dir_string.c_str());
  char* dtemp = mkdtemp(buffer);
  if (!dtemp) {
    LOG_DEBUG("mkdtemp failed");
    return false;
  }
  *new_dir = Path(dtemp);
  return true;
}

bool GetTempDir(Path* path) {
  const char* tmp = getenv("TMPDIR");
  if (tmp) {
    *path = Path(tmp);
    return true;
  }
  *path = Path("/tmp");
  return true;
}

bool CreateNewTempDirectory(const Path::BufferType& prefix, Path* new_temp_path) {
  Path tmpdir;
  if (!GetTempDir(&tmpdir))
    return false;

  return CreateTemporaryDirInDirImpl(tmpdir, GetTempTemplate(), new_temp_path);
}

bool CreateTemporaryDirInDir(const Path& base_dir,
                             const Path::BufferType& prefix,
                             Path* new_dir) {
  Path::BufferType mkdtemp_template = prefix;
  mkdtemp_template.append("XXXXXX");
  return CreateTemporaryDirInDirImpl(base_dir, Path(mkdtemp_template), new_dir);
}

bool CreateDirectoryAndGetError(const Path& full_path, File::Error* error) {
  ScopedBlockingCall scoped_blocking_call(
      FROM_HERE, BlockingType::MAY_BLOCK);  // For call to mkdir().
  base::Vector<Path> subpaths;

  // Collect a list of all parent directories.
  Path last_path = full_path;
  subpaths.push_back(full_path);
  for (Path path = full_path.DirName(); path != last_path; path = path.DirName()) {
    subpaths.push_back(path);
    last_path = path;
  }

  // Iterate through the parents and create the missing ones.
  for (const Path& subpath : base::Reversed(subpaths)) {
    if (DirectoryExists(subpath))
      continue;
    if (mkdir(subpath.c_str(), 0700) == 0)
      continue;
    // Mkdir failed, but it might have failed with EEXIST, or some other error
    // due to the directory appearing out of thin air. This can occur if
    // two processes are trying to create the same file system tree at the same
    // time. Check to see if it exists and make sure it is a directory.
    int saved_errno = errno;
    if (!DirectoryExists(subpath)) {
      if (error)
        *error = File::OSErrorToFileError(saved_errno);
      return false;
    }
  }
  return true;
}

bool CreateDirectory(const Path& full_path) {
  File::Error error = File::FILE_OK;
  return CreateDirectoryAndGetError(full_path, &error);
}

bool NormalizePath(const Path& path, Path* normalized_path) {
  Path real_path_result = MakeAbsolutePath(path);
  if (real_path_result.empty())
    return false;

  // To be consistant with windows, fail if |real_path_result| is a
  // directory.
  if (DirectoryExists(real_path_result))
    return false;

  *normalized_path = real_path_result;
  return true;
}

// TODO(rkc): Refactor GetFileInfo and FileEnumerator to handle symlinks
// correctly. http://code.google.com/p/chromium-os/issues/detail?id=15948
bool IsLink(const Path& file_path) {
  stat_wrapper_t st;
  // If we can't lstat the file, it's safe to assume that the file won't at
  // least be a 'followable' link.
  if (File::Lstat(file_path.c_str(), &st) != 0)
    return false;
  return S_ISLNK(st.st_mode);
}

bool GetFileInfo(const Path& file_path, File::Info* results) {
  stat_wrapper_t file_info;
  if (File::Stat(file_path.c_str(), &file_info) != 0)
    return false;
  results->FromStat(file_info);
  return true;
}

bool GetCurrentDirectory(Path* dir) {
  // getcwd can return ENOENT, which implies it checks against the disk.
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  char system_buffer[PATH_MAX] = "";
  if (!getcwd(system_buffer, sizeof(system_buffer))) {
    IMPOSSIBLE;
    return false;
  }
  *dir = Path(system_buffer);
  return true;
}

bool SetCurrentDirectory(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return chdir(path.c_str()) == 0;
}

int GetMaximumPathComponentLength(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return pathconf(path.c_str(), _PC_NAME_MAX);
}
}  // namespace base

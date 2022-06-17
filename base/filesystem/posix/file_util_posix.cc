// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <base/filesystem/file_util.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <base/export.h>
#include "build/build_config.h"

#if IS_LINUX
#include <sys/sendfile.h>
#endif

#include <base/filesystem/path.h>

namespace base {

namespace {

base::Path GetTempTemplate() {
  return FormatTemporaryFileName("XXXXXX");
}

bool AdvanceEnumeratorWithStat(FileEnumerator* traversal,
                               Path* out_next_path,
                               stat_wrapper_t* out_next_stat) {
  DCHECK(out_next_path);
  DCHECK(out_next_stat);
  *out_next_path = traversal->Next();
  if (out_next_path->empty())
    return false;

  *out_next_stat = traversal->GetInfo().stat();
  return true;
}

bool DoCopyDirectory(const Path& from_path,
                     const Path& to_path,
                     bool recursive,
                     bool open_exclusive) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  // Some old callers of CopyDirectory want it to support wildcards.
  // After some discussion, we decided to fix those callers.
  // Break loudly here if anyone tries to do this.
  DCHECK(to_path.value().find('*') == std::string::npos);
  DCHECK(from_path.value().find('*') == std::string::npos);

  if (from_path.value().size() >= PATH_MAX) {
    return false;
  }

  // This function does not properly handle destinations within the source
  Path real_to_path = to_path;
  if (PathExists(real_to_path))
    real_to_path = MakeAbsolutePath(real_to_path);
  else
    real_to_path = MakeAbsolutePath(real_to_path.DirName());
  if (real_to_path.empty())
    return false;

  Path real_from_path = MakeAbsolutePath(from_path);
  if (real_from_path.empty())
    return false;
  if (real_to_path == real_from_path || real_from_path.IsParent(real_to_path))
    return false;

  int traverse_type = FileEnumerator::FILES | FileEnumerator::SHOW_SYM_LINKS;
  if (recursive)
    traverse_type |= FileEnumerator::DIRECTORIES;
  FileEnumerator traversal(from_path, recursive, traverse_type);

  // We have to mimic windows behavior here. |to_path| may not exist yet,
  // start the loop with |to_path|.
  stat_wrapper_t from_stat;
  Path current = from_path;
  if (File::Stat(from_path.value().c_str(), &from_stat) < 0) {
    DPLOG(ERROR) << "CopyDirectory() couldn't stat source directory: "
                 << from_path.value();
    return false;
  }
  Path from_path_base = from_path;
  if (recursive && DirectoryExists(to_path)) {
    // If the destination already exists and is a directory, then the
    // top level of source needs to be copied.
    from_path_base = from_path.DirName();
  }

  // The Windows version of this function assumes that non-recursive calls
  // will always have a directory for from_path.
  // TODO(maruel): This is not necessary anymore.
  DCHECK(recursive || S_ISDIR(from_stat.st_mode));

  do {
    // current is the source path, including from_path, so append
    // the suffix after from_path to to_path to create the target_path.
    Path target_path(to_path);
    if (from_path_base != current &&
        !from_path_base.AppendRelativePath(current, &target_path)) {
      return false;
    }

    if (S_ISDIR(from_stat.st_mode)) {
      mode_t mode = (from_stat.st_mode & 01777) | S_IRUSR | S_IXUSR | S_IWUSR;
      if (mkdir(target_path.value().c_str(), mode) == 0)
        continue;
      if (errno == EEXIST && !open_exclusive)
        continue;

      DPLOG(ERROR) << "CopyDirectory() couldn't create directory: "
                   << target_path.value();
      return false;
    }

    if (!S_ISREG(from_stat.st_mode)) {
      DLOG(WARNING) << "CopyDirectory() skipping non-regular file: "
                    << current.value();
      continue;
    }

    // Add O_NONBLOCK so we can't block opening a pipe.
    File infile(open(current.value().c_str(), O_RDONLY | O_NONBLOCK));
    if (!infile.IsValid()) {
      DPLOG(ERROR) << "CopyDirectory() couldn't open file: " << current.value();
      return false;
    }

    stat_wrapper_t stat_at_use;
    if (File::Fstat(infile.GetPlatformFile(), &stat_at_use) < 0) {
      DPLOG(ERROR) << "CopyDirectory() couldn't stat file: " << current.value();
      return false;
    }

    if (!S_ISREG(stat_at_use.st_mode)) {
      DLOG(WARNING) << "CopyDirectory() skipping non-regular file: "
                    << current.value();
      continue;
    }

    int open_flags = O_WRONLY | O_CREAT;
    // If |open_exclusive| is set then we should always create the destination
    // file, so O_NONBLOCK is not necessary to ensure we don't block on the
    // open call for the target file below, and since the destination will
    // always be a regular file it wouldn't affect the behavior of the
    // subsequent write calls anyway.
    if (open_exclusive)
      open_flags |= O_EXCL;
    else
      open_flags |= O_TRUNC | O_NONBLOCK;
    // Each platform has different default file opening modes for CopyFile which
    // we want to replicate here. On OS X, we use copyfile(3) which takes the
    // source file's permissions into account. On the other platforms, we just
    // use the base::File constructor. On Chrome OS, base::File uses a different
    // set of permissions than it does on other POSIX platforms.
#if BUILDFLAG(IS_APPLE)
    int mode = 0600 | (stat_at_use.st_mode & 0177);
#elif BUILDFLAG(IS_CHROMEOS)
    int mode = 0644;
#else
    int mode = 0600;
#endif
    File outfile(open(target_path.value().c_str(), open_flags, mode));
    if (!outfile.IsValid()) {
      DPLOG(ERROR) << "CopyDirectory() couldn't create file: "
                   << target_path.value();
      return false;
    }

    if (!CopyFileContents(infile, outfile)) {
      DLOG(ERROR) << "CopyDirectory() couldn't copy file: " << current.value();
      return false;
    }
  } while (AdvanceEnumeratorWithStat(&traversal, &current, &from_stat));

  return true;
}

// TODO(erikkay): The Windows version of this accepts paths like "foo/bar/*"
// which works both with and without the recursive flag.  I'm not sure we need
// that functionality. If not, remove from file_util_win.cc, otherwise add it
// here.
bool DoDeleteFile(const Path& path, bool recursive) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

#if BUILDFLAG(IS_ANDROID)
  if (path.IsContentUri())
    return DeleteContentUri(path);
#endif  // BUILDFLAG(IS_ANDROID)

  const char* path_str = path.value().c_str();
  stat_wrapper_t file_info;
  if (File::Lstat(path_str, &file_info) != 0) {
    // The Windows version defines this condition as success.
    return (errno == ENOENT);
  }
  if (!S_ISDIR(file_info.st_mode))
    return (unlink(path_str) == 0) || (errno == ENOENT);
  if (!recursive)
    return (rmdir(path_str) == 0) || (errno == ENOENT);

  bool success = true;
  stack<std::string> directories;
  directories.push(path.value());
  FileEnumerator traversal(path, true,
      FileEnumerator::FILES | FileEnumerator::DIRECTORIES |
      FileEnumerator::SHOW_SYM_LINKS);
  for (Path current = traversal.Next(); !current.empty();
       current = traversal.Next()) {
    if (traversal.GetInfo().IsDirectory())
      directories.push(current.value());
    else
      success &= (unlink(current.value().c_str()) == 0) || (errno == ENOENT);
  }

  while (!directories.empty()) {
    Path dir = Path(directories.top());
    directories.pop();
    success &= (rmdir(dir.value().c_str()) == 0) || (errno == ENOENT);
  }
  return success;
}

#if !BUILDFLAG(IS_APPLE)
// Appends |mode_char| to |mode| before the optional character set encoding; see
// https://www.gnu.org/software/libc/manual/html_node/Opening-Streams.html for
// details.
std::string AppendModeCharacter(StringPiece mode, char mode_char) {
  std::string result(mode);
  size_t comma_pos = result.find(',');
  result.insert(comma_pos == std::string::npos ? result.length() : comma_pos, 1,
                mode_char);
  return result;
}
#endif

}  // namespace

Path MakeAbsolutePath(const Path& input) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  char full_path[PATH_MAX];
  if (realpath(input.value().c_str(), full_path) == nullptr)
    return Path();
  return Path(full_path);
}

bool DeleteFile(const Path& path) {
  return DoDeleteFile(path, /*recursive=*/false);
}

bool DeletePathRecursively(const Path& path) {
  return DoDeleteFile(path, /*recursive=*/true);
}

bool ReplaceFile(const Path& from_path,
                 const Path& to_path,
                 File::Error* error) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (rename(from_path.value().c_str(), to_path.value().c_str()) == 0)
    return true;
  if (error)
    *error = File::GetLastFileError();
  return false;
}

bool CopyDirectory(const Path& from_path,
                   const Path& to_path,
                   bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, false);
}

bool CopyDirectoryExcl(const Path& from_path,
                       const Path& to_path,
                       bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, true);
}

bool CreatePipe(ScopedFD* read_fd, ScopedFD* write_fd, bool non_blocking) {
  int fds[2];
  bool created =
      non_blocking ? CreateLocalNonBlockingPipe(fds) : (0 == pipe(fds));
  if (!created)
    return false;
  read_fd->reset(fds[0]);
  write_fd->reset(fds[1]);
  return true;
}

bool CreateLocalNonBlockingPipe(int fds[2]) {
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  return pipe2(fds, O_CLOEXEC | O_NONBLOCK) == 0;
#else
  int raw_fds[2];
  if (pipe(raw_fds) != 0)
    return false;
  ScopedFD fd_out(raw_fds[0]);
  ScopedFD fd_in(raw_fds[1]);
  if (!SetCloseOnExec(fd_out.get()))
    return false;
  if (!SetCloseOnExec(fd_in.get()))
    return false;
  if (!SetNonBlocking(fd_out.get()))
    return false;
  if (!SetNonBlocking(fd_in.get()))
    return false;
  fds[0] = fd_out.release();
  fds[1] = fd_in.release();
  return true;
#endif
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
#if BUILDFLAG(IS_ANDROID)
  if (path.IsContentUri()) {
    return ContentUriExists(path);
  }
#endif
  return access(path.value().c_str(), F_OK) == 0;
}

bool DirectoryExists(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  stat_wrapper_t file_info;
  if (File::Stat(path.value().c_str(), &file_info) != 0)
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

ScopedFD CreateAndOpenFdForTemporaryFileInDir(const Path& directory,
                                              Path* path) {
  ScopedBlockingCall scoped_blocking_call(
      FROM_HERE,
      BlockingType::MAY_BLOCK);  // For call to mkstemp().
  *path = directory.Append(GetTempTemplate());
  const std::string& tmpdir_string = path->value();
  // this should be OK since mkstemp just replaces characters in place
  char* buffer = const_cast<char*>(tmpdir_string.c_str());

  return ScopedFD(HANDLE_EINTR(mkstemp(buffer)));
}

#if !BUILDFLAG(IS_FUCHSIA)
bool CreateSymbolicLink(const Path& target_path,
                        const Path& symlink_path) {
  DCHECK(!symlink_path.empty());
  DCHECK(!target_path.empty());
  return ::symlink(target_path.value().c_str(),
                   symlink_path.value().c_str()) != -1;
}

bool ReadSymbolicLink(const Path& symlink_path, Path* target_path) {
  DCHECK(!symlink_path.empty());
  DCHECK(target_path);
  char buf[PATH_MAX];
  ssize_t count = ::readlink(symlink_path.value().c_str(), buf, std::size(buf));

#if BUILDFLAG(IS_ANDROID) && defined(__LP64__)
  // A few 64-bit Android L/M devices return INT_MAX instead of -1 here for
  // errors; this is related to bionic's (incorrect) definition of ssize_t as
  // being long int instead of int. Cast it so the compiler generates the
  // comparison we want here. https://crbug.com/1101940
  bool error = static_cast<int32_t>(count) <= 0;
#else
  bool error = count <= 0;
#endif

  if (error) {
    target_path->clear();
    return false;
  }

  *target_path = Path(Path::StringType(buf, count));
  return true;
}

bool GetPosixFilePermissions(const Path& path, int* mode) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(mode);

  stat_wrapper_t file_info;
  // Uses stat(), because on symbolic link, lstat() does not return valid
  // permission bits in st_mode
  if (File::Stat(path.value().c_str(), &file_info) != 0)
    return false;

  *mode = file_info.st_mode & FILE_PERMISSION_MASK;
  return true;
}

bool SetPosixFilePermissions(const Path& path,
                             int mode) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK_EQ(mode & ~FILE_PERMISSION_MASK, 0);

  // Calls stat() so that we can preserve the higher bits like S_ISGID.
  stat_wrapper_t stat_buf;
  if (File::Stat(path.value().c_str(), &stat_buf) != 0)
    return false;

  // Clears the existing permission bits, and adds the new ones.
  mode_t updated_mode_bits = stat_buf.st_mode & ~FILE_PERMISSION_MASK;
  updated_mode_bits |= mode & FILE_PERMISSION_MASK;

  if (HANDLE_EINTR(chmod(path.value().c_str(), updated_mode_bits)) != 0)
    return false;

  return true;
}

bool ExecutableExistsInPath(Environment* env,
                            const Path::StringType& executable) {
  std::string path;
  if (!env->GetVar("PATH", &path)) {
    LOG(ERROR) << "No $PATH variable. Assuming no " << executable << ".";
    return false;
  }

  for (const StringPiece& cur_path :
       SplitStringPiece(path, ":", KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY)) {
    Path file(cur_path);
    int permissions;
    if (GetPosixFilePermissions(file.Append(executable), &permissions) &&
        (permissions & FILE_PERMISSION_EXECUTE_BY_USER))
      return true;
  }
  return false;
}

#endif  // !BUILDFLAG(IS_FUCHSIA)

#if !BUILDFLAG(IS_APPLE)
// This is implemented in file_util_mac.mm for Mac.
bool GetTempDir(Path* path) {
  const char* tmp = getenv("TMPDIR");
  if (tmp) {
    *path = Path(tmp);
    return true;
  }

#if BUILDFLAG(IS_ANDROID)
  return PathService::Get(DIR_CACHE, path);
#else
  *path = Path("/tmp");
  return true;
#endif
}
#endif  // !BUILDFLAG(IS_APPLE)

#if !BUILDFLAG(IS_APPLE)  // Mac implementation is in file_util_mac.mm.
Path GetHomeDir() {
#if BUILDFLAG(IS_CHROMEOS)
  if (SysInfo::IsRunningOnChromeOS()) {
    // On Chrome OS chrome::DIR_USER_DATA is overridden with a primary user
    // homedir once it becomes available. Return / as the safe option.
    return Path("/");
  }
#endif

  const char* home_dir = getenv("HOME");
  if (home_dir && home_dir[0])
    return Path(home_dir);

#if BUILDFLAG(IS_ANDROID)
  DLOG(WARNING) << "OS_ANDROID: Home directory lookup not yet implemented.";
#endif

  Path rv;
  if (GetTempDir(&rv))
    return rv;

  // Last resort.
  return Path("/tmp");
}
#endif  // !BUILDFLAG(IS_APPLE)

File CreateAndOpenTemporaryFileInDir(const Path& dir, Path* temp_file) {
  // For call to close() inside ScopedFD.
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  ScopedFD fd = CreateAndOpenFdForTemporaryFileInDir(dir, temp_file);
  return fd.is_valid() ? File(std::move(fd)) : File(File::GetLastFileError());
}

bool CreateTemporaryFileInDir(const Path& dir, Path* temp_file) {
  // For call to close() inside ScopedFD.
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  ScopedFD fd = CreateAndOpenFdForTemporaryFileInDir(dir, temp_file);
  return fd.is_valid();
}

Path FormatTemporaryFileName(Path::StringPieceType identifier) {
#if BUILDFLAG(IS_APPLE)
  StringPiece prefix = base::mac::BaseBundleID();
#elif BUILDFLAG(GOOGLE_CHROME_BRANDING)
  StringPiece prefix = "com.google.Chrome";
#else
  StringPiece prefix = "org.chromium.Chromium";
#endif
  return Path(StrCat({".", prefix, ".", identifier}));
}

ScopedFILE CreateAndOpenTemporaryStreamInDir(const Path& dir,
                                             Path* path) {
  ScopedFD scoped_fd = CreateAndOpenFdForTemporaryFileInDir(dir, path);
  if (!scoped_fd.is_valid())
    return nullptr;

  int fd = scoped_fd.release();
  FILE* file = fdopen(fd, "a+");
  if (!file)
    close(fd);
  return ScopedFILE(file);
}

static bool CreateTemporaryDirInDirImpl(const Path& base_dir,
                                        const Path& name_tmpl,
                                        Path* new_dir) {
  ScopedBlockingCall scoped_blocking_call(
      FROM_HERE, BlockingType::MAY_BLOCK);  // For call to mkdtemp().
  DCHECK(EndsWith(name_tmpl.value(), "XXXXXX"))
      << "Directory name template must end with \"XXXXXX\".";

  Path sub_dir = base_dir.Append(name_tmpl);
  std::string sub_dir_string = sub_dir.value();

  // this should be OK since mkdtemp just replaces characters in place
  char* buffer = const_cast<char*>(sub_dir_string.c_str());
  char* dtemp = mkdtemp(buffer);
  if (!dtemp) {
    DPLOG(ERROR) << "mkdtemp";
    return false;
  }
  *new_dir = Path(dtemp);
  return true;
}

bool CreateTemporaryDirInDir(const Path& base_dir,
                             const Path::StringType& prefix,
                             Path* new_dir) {
  Path::StringType mkdtemp_template = prefix;
  mkdtemp_template.append("XXXXXX");
  return CreateTemporaryDirInDirImpl(base_dir, Path(mkdtemp_template),
                                     new_dir);
}

bool CreateNewTempDirectory(const Path::StringType& prefix,
                            Path* new_temp_path) {
  Path tmpdir;
  if (!GetTempDir(&tmpdir))
    return false;

  return CreateTemporaryDirInDirImpl(tmpdir, GetTempTemplate(), new_temp_path);
}

bool CreateDirectoryAndGetError(const Path& full_path,
                                File::Error* error) {
  ScopedBlockingCall scoped_blocking_call(
      FROM_HERE, BlockingType::MAY_BLOCK);  // For call to mkdir().
  std::vector<Path> subpaths;

  // Collect a list of all parent directories.
  Path last_path = full_path;
  subpaths.push_back(full_path);
  for (Path path = full_path.DirName();
       path.value() != last_path.value(); path = path.DirName()) {
    subpaths.push_back(path);
    last_path = path;
  }

  // Iterate through the parents and create the missing ones.
  for (const Path& subpath : base::Reversed(subpaths)) {
    if (DirectoryExists(subpath))
      continue;
    if (mkdir(subpath.value().c_str(), 0700) == 0)
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

// ReadFileToStringNonBlockingNonBlocking will read a file to a string. This
// method should only be used on files which are known to be non-blocking such
// as procfs or sysfs nodes. Additionally, the file is opened as O_NONBLOCK so
// it WILL NOT block even if opened on a blocking file. It will return true if
// the file read until EOF and it will return false otherwise, errno will remain
// set on error conditions. |ret| will be populated with the contents of the
// file.
bool ReadFileToStringNonBlocking(const base::Path& file, std::string* ret) {
  DCHECK(ret);
  ret->clear();

  base::ScopedFD fd(HANDLE_EINTR(
      open(file.MaybeAsASCII().c_str(), O_CLOEXEC | O_NONBLOCK | O_RDONLY)));
  if (!fd.is_valid()) {
    return false;
  }

  ssize_t bytes_read = 0;
  do {
    char buf[4096];
    bytes_read = HANDLE_EINTR(read(fd.get(), buf, sizeof(buf)));
    if (bytes_read < 0) {
      return false;
    } else if (bytes_read > 0) {
      ret->append(buf, bytes_read);
    }
  } while (bytes_read > 0);

  return true;
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
  if (File::Lstat(file_path.value().c_str(), &st) != 0)
    return false;
  return S_ISLNK(st.st_mode);
}

bool GetFileInfo(const Path& file_path, File::Info* results) {
  stat_wrapper_t file_info;
#if BUILDFLAG(IS_ANDROID)
  if (file_path.IsContentUri()) {
    File file = OpenContentUriForRead(file_path);
    if (!file.IsValid())
      return false;
    return file.GetInfo(results);
  } else {
#endif  // BUILDFLAG(IS_ANDROID)
    if (File::Stat(file_path.value().c_str(), &file_info) != 0)
      return false;
#if BUILDFLAG(IS_ANDROID)
  }
#endif  // BUILDFLAG(IS_ANDROID)

  results->FromStat(file_info);
  return true;
}

FILE* OpenFile(const Path& filename, const char* mode) {
  // 'e' is unconditionally added below, so be sure there is not one already
  // present before a comma in |mode|.
  DCHECK(
      strchr(mode, 'e') == nullptr ||
      (strchr(mode, ',') != nullptr && strchr(mode, 'e') > strchr(mode, ',')));
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  FILE* result = nullptr;
#if BUILDFLAG(IS_APPLE)
  // macOS does not provide a mode character to set O_CLOEXEC; see
  // https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man3/fopen.3.html.
  const char* the_mode = mode;
#else
  std::string mode_with_e(AppendModeCharacter(mode, 'e'));
  const char* the_mode = mode_with_e.c_str();
#endif
  do {
    result = fopen(filename.value().c_str(), the_mode);
  } while (!result && errno == EINTR);
#if BUILDFLAG(IS_APPLE)
  // Mark the descriptor as close-on-exec.
  if (result)
    SetCloseOnExec(fileno(result));
#endif
  return result;
}

// NaCl doesn't implement system calls to open files directly.
#if !BUILDFLAG(IS_NACL)
FILE* FileToFILE(File file, const char* mode) {
  PlatformFile unowned = file.GetPlatformFile();
  FILE* stream = fdopen(file.TakePlatformFile(), mode);
  if (!stream)
    ScopedFD to_be_closed(unowned);
  return stream;
}

File FILEToFile(FILE* file_stream) {
  if (!file_stream)
    return File();

  PlatformFile fd = fileno(file_stream);
  DCHECK_NE(fd, -1);
  ScopedPlatformFile other_fd(HANDLE_EINTR(dup(fd)));
  if (!other_fd.is_valid())
    return File(File::GetLastFileError());
  return File(std::move(other_fd));
}
#endif  // !BUILDFLAG(IS_NACL)

int ReadFile(const Path& filename, char* data, int max_size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  int fd = HANDLE_EINTR(open(filename.value().c_str(), O_RDONLY));
  if (fd < 0)
    return -1;

  ssize_t bytes_read = HANDLE_EINTR(read(fd, data, max_size));
  if (IGNORE_EINTR(close(fd)) < 0)
    return -1;
  return bytes_read;
}

int WriteFile(const Path& filename, const char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  int fd = HANDLE_EINTR(creat(filename.value().c_str(), 0666));
  if (fd < 0)
    return -1;

  int bytes_written =
      WriteFileDescriptor(fd, StringPiece(data, size)) ? size : -1;
  if (IGNORE_EINTR(close(fd)) < 0)
    return -1;
  return bytes_written;
}

bool WriteFileDescriptor(int fd, span<const uint8_t> data) {
  // Allow for partial writes.
  ssize_t bytes_written_total = 0;
  ssize_t size = checked_cast<ssize_t>(data.size());
  for (ssize_t bytes_written_partial = 0; bytes_written_total < size;
       bytes_written_total += bytes_written_partial) {
    bytes_written_partial = HANDLE_EINTR(write(
        fd, data.data() + bytes_written_total, size - bytes_written_total));
    if (bytes_written_partial < 0)
      return false;
  }

  return true;
}

bool WriteFileDescriptor(int fd, StringPiece data) {
  return WriteFileDescriptor(fd, as_bytes(make_span(data)));
}

bool AllocateFileRegion(File* file, int64_t offset, size_t size) {
  DCHECK(file);

  // Explicitly extend |file| to the maximum size. Zeros will fill the new
  // space. It is assumed that the existing file is fully realized as
  // otherwise the entire file would have to be read and possibly written.
  const int64_t original_file_len = file->GetLength();
  if (original_file_len < 0) {
    DPLOG(ERROR) << "fstat " << file->GetPlatformFile();
    return false;
  }

  // Increase the actual length of the file, if necessary. This can fail if
  // the disk is full and the OS doesn't support sparse files.
  const int64_t new_file_len = offset + size;
  if (!file->SetLength(std::max(original_file_len, new_file_len))) {
    DPLOG(ERROR) << "ftruncate " << file->GetPlatformFile();
    return false;
  }

  // Realize the extent of the file so that it can't fail (and crash) later
  // when trying to write to a memory page that can't be created. This can
  // fail if the disk is full and the file is sparse.

  // First try the more effective platform-specific way of allocating the disk
  // space. It can fail because the filesystem doesn't support it. In that case,
  // use the manual method below.

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  if (HANDLE_EINTR(fallocate(file->GetPlatformFile(), 0, offset, size)) != -1)
    return true;
  DPLOG(ERROR) << "fallocate";
#elif BUILDFLAG(IS_APPLE)
  // MacOS doesn't support fallocate even though their new APFS filesystem
  // does support sparse files. It does, however, have the functionality
  // available via fcntl.
  // See also: https://openradar.appspot.com/32720223
  fstore_t params = {F_ALLOCATEALL, F_PEOFPOSMODE, offset,
                     static_cast<off_t>(size), 0};
  if (fcntl(file->GetPlatformFile(), F_PREALLOCATE, &params) != -1)
    return true;
  DPLOG(ERROR) << "F_PREALLOCATE";
#endif

  // Manually realize the extended file by writing bytes to it at intervals.
  int64_t block_size = 512;  // Start with something safe.
  stat_wrapper_t statbuf;
  if (File::Fstat(file->GetPlatformFile(), &statbuf) == 0 &&
      statbuf.st_blksize > 0 && base::bits::IsPowerOfTwo(statbuf.st_blksize)) {
    block_size = statbuf.st_blksize;
  }

  // Write starting at the next block boundary after the old file length.
  const int64_t extension_start =
      base::bits::AlignUp(original_file_len, block_size);
  for (int64_t i = extension_start; i < new_file_len; i += block_size) {
    char existing_byte;
    if (HANDLE_EINTR(pread(file->GetPlatformFile(), &existing_byte, 1, i)) !=
        1) {
      return false;  // Can't read? Not viable.
    }
    if (existing_byte != 0) {
      continue;  // Block has data so must already exist.
    }
    if (HANDLE_EINTR(pwrite(file->GetPlatformFile(), &existing_byte, 1, i)) !=
        1) {
      return false;  // Can't write? Not viable.
    }
  }

  return true;
}

bool AppendToFile(const Path& filename, span<const uint8_t> data) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  bool ret = true;
  int fd = HANDLE_EINTR(open(filename.value().c_str(), O_WRONLY | O_APPEND));
  if (fd < 0) {
    VPLOG(1) << "Unable to create file " << filename.value();
    return false;
  }

  // This call will either write all of the data or return false.
  if (!WriteFileDescriptor(fd, data)) {
    VPLOG(1) << "Error while writing to file " << filename.value();
    ret = false;
  }

  if (IGNORE_EINTR(close(fd)) < 0) {
    VPLOG(1) << "Error while closing file " << filename.value();
    return false;
  }

  return ret;
}

bool AppendToFile(const Path& filename, StringPiece data) {
  return AppendToFile(filename, as_bytes(make_span(data)));
}

bool GetCurrentDirectory(Path* dir) {
  // getcwd can return ENOENT, which implies it checks against the disk.
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  char system_buffer[PATH_MAX] = "";
  if (!getcwd(system_buffer, sizeof(system_buffer))) {
    NOTREACHED();
    return false;
  }
  *dir = Path(system_buffer);
  return true;
}

bool SetCurrentDirectory(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return chdir(path.value().c_str()) == 0;
}

#if BUILDFLAG(IS_MAC)
bool VerifyPathControlledByUser(const Path& base,
                                const Path& path,
                                uid_t owner_uid,
                                const std::set<gid_t>& group_gids) {
  if (base != path && !base.IsParent(path)) {
     DLOG(ERROR) << "|base| must be a subdirectory of |path|.  base = \""
                 << base.value() << "\", path = \"" << path.value() << "\"";
     return false;
  }

  std::vector<Path::StringType> base_components = base.GetComponents();
  std::vector<Path::StringType> path_components = path.GetComponents();
  std::vector<Path::StringType>::const_iterator ib, ip;
  for (ib = base_components.begin(), ip = path_components.begin();
       ib != base_components.end(); ++ib, ++ip) {
    // |base| must be a subpath of |path|, so all components should match.
    // If these CHECKs fail, look at the test that base is a parent of
    // path at the top of this function.
    DCHECK(ip != path_components.end());
    DCHECK(*ip == *ib);
  }

  Path current_path = base;
  if (!VerifySpecificPathControlledByUser(current_path, owner_uid, group_gids))
    return false;

  for (; ip != path_components.end(); ++ip) {
    current_path = current_path.Append(*ip);
    if (!VerifySpecificPathControlledByUser(
            current_path, owner_uid, group_gids))
      return false;
  }
  return true;
}

bool VerifyPathControlledByAdmin(const Path& path) {
  const unsigned kRootUid = 0;
  const Path kFileSystemRoot("/");

  // The name of the administrator group on mac os.
  const char* const kAdminGroupNames[] = {
    "admin",
    "wheel"
  };

  // Reading the groups database may touch the file system.
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  std::set<gid_t> allowed_group_ids;
  for (int i = 0, ie = std::size(kAdminGroupNames); i < ie; ++i) {
    struct group *group_record = getgrnam(kAdminGroupNames[i]);
    if (!group_record) {
      DPLOG(ERROR) << "Could not get the group ID of group \""
                   << kAdminGroupNames[i] << "\".";
      continue;
    }

    allowed_group_ids.insert(group_record->gr_gid);
  }

  return VerifyPathControlledByUser(
      kFileSystemRoot, path, kRootUid, allowed_group_ids);
}
#endif  // BUILDFLAG(IS_MAC)

int GetMaximumPathComponentLength(const Path& path) {
#if BUILDFLAG(IS_FUCHSIA)
  // Return a value we do not expect anyone ever to reach, but which is small
  // enough to guard against e.g. bugs causing multi-megabyte paths.
  return 1024;
#else
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return pathconf(path.value().c_str(), _PC_NAME_MAX);
#endif
}

#if !BUILDFLAG(IS_ANDROID)
// This is implemented in file_util_android.cc for that platform.
bool GetShmemTempDir(bool executable, Path* path) {
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_AIX)
  bool disable_dev_shm = false;
#if !BUILDFLAG(IS_CHROMEOS)
  disable_dev_shm = CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kDisableDevShmUsage);
#endif
  bool use_dev_shm = true;
  if (executable) {
    static const bool s_dev_shm_executable =
        IsPathExecutable(Path("/dev/shm"));
    use_dev_shm = s_dev_shm_executable;
  }
  if (use_dev_shm && !disable_dev_shm) {
    *path = Path("/dev/shm");
    return true;
  }
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_AIX)
  return GetTempDir(path);
}
#endif  // !BUILDFLAG(IS_ANDROID)

#if !BUILDFLAG(IS_APPLE)
// Mac has its own implementation, this is for all other Posix systems.
bool CopyFile(const Path& from_path, const Path& to_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  File infile;
#if BUILDFLAG(IS_ANDROID)
  if (from_path.IsContentUri()) {
    infile = OpenContentUriForRead(from_path);
  } else {
    infile = File(from_path, File::FLAG_OPEN | File::FLAG_READ);
  }
#else
  infile = File(from_path, File::FLAG_OPEN | File::FLAG_READ);
#endif
  if (!infile.IsValid())
    return false;

  File outfile(to_path, File::FLAG_WRITE | File::FLAG_CREATE_ALWAYS);
  if (!outfile.IsValid())
    return false;

  return CopyFileContents(infile, outfile);
}
#endif  // !BUILDFLAG(IS_APPLE)

bool PreReadFile(const Path& file_path,
                 bool is_executable,
                 int64_t max_bytes) {
  DCHECK_GE(max_bytes, 0);

  // posix_fadvise() is only available in the Android NDK in API 21+. Older
  // versions may have the required kernel support, but don't have enough usage
  // to justify backporting.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || \
    (BUILDFLAG(IS_ANDROID) && __ANDROID_API__ >= 21)
  File file(file_path, File::FLAG_OPEN | File::FLAG_READ);
  if (!file.IsValid())
    return false;

  if (max_bytes == 0) {
    // fadvise() pre-fetches the entire file when given a zero length.
    return true;
  }

  const PlatformFile fd = file.GetPlatformFile();
  const ::off_t len = base::saturated_cast<::off_t>(max_bytes);
  return posix_fadvise(fd, /*offset=*/0, len, POSIX_FADV_WILLNEED) == 0;
#elif BUILDFLAG(IS_APPLE)
  File file(file_path, File::FLAG_OPEN | File::FLAG_READ);
  if (!file.IsValid())
    return false;

  if (max_bytes == 0) {
    // fcntl(F_RDADVISE) fails when given a zero length.
    return true;
  }

  const PlatformFile fd = file.GetPlatformFile();
  ::radvisory read_advise_data = {
      .ra_offset = 0, .ra_count = base::saturated_cast<int>(max_bytes)};
  return fcntl(fd, F_RDADVISE, &read_advise_data) != -1;
#else
  return internal::PreReadFileSlow(file_path, max_bytes);
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) ||
        // (BUILDFLAG(IS_ANDROID) &&
        // __ANDROID_API__ >= 21)
}

// -----------------------------------------------------------------------------

namespace internal {

bool MoveUnsafe(const Path& from_path, const Path& to_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  // Windows compatibility: if |to_path| exists, |from_path| and |to_path|
  // must be the same type, either both files, or both directories.
  stat_wrapper_t to_file_info;
  if (File::Stat(to_path.value().c_str(), &to_file_info) == 0) {
    stat_wrapper_t from_file_info;
    if (File::Stat(from_path.value().c_str(), &from_file_info) != 0)
      return false;
    if (S_ISDIR(to_file_info.st_mode) != S_ISDIR(from_file_info.st_mode))
      return false;
  }

  if (rename(from_path.value().c_str(), to_path.value().c_str()) == 0)
    return true;

  if (!CopyDirectory(from_path, to_path, true))
    return false;

  DeletePathRecursively(from_path);
  return true;
}

bool CopyFileContentsWithSendfile(File& infile,
                                  File& outfile,
                                  bool& retry_slow) {
  DCHECK(infile.IsValid());
  stat_wrapper_t in_file_info;
  retry_slow = false;

  if (base::File::Fstat(infile.GetPlatformFile(), &in_file_info)) {
    return false;
  }

  int64_t file_size = in_file_info.st_size;
  if (file_size == 0) {
    // Non-regular files can return a file size of 0, things such as pipes,
    // sockets, etc. Additionally, kernel seq_files(most procfs files) will also
    // return 0 while still reporting as a regular file. Unfortunately, in some
    // of these situations there are easy ways to detect them, in others there
    // are not. No extra syscalls are needed if it's not a regular file.
    //
    // Because any attempt to detect it would likely require another syscall,
    // let's just fall back to a slow copy which will invoke a single read(2) to
    // determine if the file has contents or if it's really a zero length file.
    retry_slow = true;
    return false;
  }

  size_t copied = 0;
  ssize_t res = 0;
  while (file_size - copied > 0) {
    // Don't specify an offset and the kernel will begin reading/writing to the
    // current file offsets.
    res = HANDLE_EINTR(sendfile(outfile.GetPlatformFile(),
                                infile.GetPlatformFile(), /*offset=*/nullptr,
                                /*length=*/file_size - copied));
    if (res <= 0) {
      break;
    }

    copied += res;
  }

  // Fallback on non-fatal error cases. None of these errors can happen after
  // data has started copying, a check is included for good measure. As a result
  // file sizes and file offsets will not have changed. A slow fallback and
  // proceed without issues.
  retry_slow = (copied == 0 && res < 0 &&
                (errno == EINVAL || errno == ENOSYS || errno == EPERM));

  return res >= 0;
}
}  // namespace internal

BASE_EXPORT bool IsPathExecutable(const Path& path) {
  bool result = false;
  Path tmp_file_path;

  ScopedFD fd = CreateAndOpenFdForTemporaryFileInDir(path, &tmp_file_path);
  if (fd.is_valid()) {
    DeleteFile(tmp_file_path);
    long sysconf_result = sysconf(_SC_PAGESIZE);
    CHECK_GE(sysconf_result, 0);
    size_t pagesize = static_cast<size_t>(sysconf_result);
    CHECK_GE(sizeof(pagesize), sizeof(sysconf_result));
    void* mapping = mmap(nullptr, pagesize, PROT_READ, MAP_SHARED, fd.get(), 0);
    if (mapping != MAP_FAILED) {
      if (HANDLE_EINTR(mprotect(mapping, pagesize, PROT_READ | PROT_EXEC)) == 0)
        result = true;
      munmap(mapping, pagesize);
    }
  }
  return result;
}
}  // namespace base

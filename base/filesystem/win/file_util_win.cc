// Copyright(C) 2021 Force67<github.com / Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "base/filesystem/file_util.h"

#include "base/random.h"
#include "base/logging.h"

#define FMT_ENABLE_WIDE
#include <fmt/format.h>
#include <windows.h>
#include <base/threading/scoped_blocking_call.h>

namespace base {

bool CreateTemporaryDirInDir(const Path& base_dir,
                             const Path::BufferType& prefix,
                             // TODO: prefix is currently ignored
                             Path* new_dir) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  Path path_to_create;
  for (int count = 0; count < 50; ++count) {
    // TODO(Vince): numeric conv functions
    auto name =
        fmt::format("scoped_dir_{}_{}", ::GetCurrentProcessId(), RandomUint());

    path_to_create = base_dir / name.c_str();
    if (::CreateDirectoryW(path_to_create.c_str(), nullptr)) {
      *new_dir = path_to_create;
      return true;
    }
  }

  return false;
}

bool DirectoryExists(const Path& path) {
  DWORD fileattr = ::GetFileAttributesW(path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

bool PathExists(const Path& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return (::GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
}

DWORD ReturnLastErrorOrSuccessOnNotFound() {
  const DWORD error_code = ::GetLastError();
  return (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND)
             ? ERROR_SUCCESS
             : error_code;
}

// Deletes all files and directories in a path.
// Returns ERROR_SUCCESS on success or the Windows error code corresponding to
// the first error encountered. ERROR_FILE_NOT_FOUND and ERROR_PATH_NOT_FOUND
// are considered success conditions, and are therefore never returned.
DWORD DeleteFileRecursive(const Path& path,
                          const Path::BufferType& pattern,
                          bool recursive) {
    #if 0
  FileEnumerator traversal(
      path, false, FileEnumerator::FILES | FileEnumerator::DIRECTORIES, pattern);
  DWORD result = ERROR_SUCCESS;
  for (FilePath current = traversal.Next(); !current.empty();
       current = traversal.Next()) {
    // Try to clear the read-only bit if we find it.
    FileEnumerator::FileInfo info = traversal.GetInfo();
    if ((info.find_data().dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        (recursive || !info.IsDirectory())) {
      ::SetFileAttributes(
          current.value().c_str(),
          info.find_data().dwFileAttributes & ~DWORD{FILE_ATTRIBUTE_READONLY});
    }

    DWORD this_result = ERROR_SUCCESS;
    if (info.IsDirectory()) {
      if (recursive) {
        this_result = DeleteFileRecursive(current, pattern, true);
        DCHECK_NE(static_cast<LONG>(this_result), ERROR_FILE_NOT_FOUND);
        DCHECK_NE(static_cast<LONG>(this_result), ERROR_PATH_NOT_FOUND);
        if (this_result == ERROR_SUCCESS &&
            !::RemoveDirectory(current.value().c_str())) {
          this_result = ReturnLastErrorOrSuccessOnNotFound();
        }
      }
    } else if (!::DeleteFile(current.value().c_str())) {
      this_result = ReturnLastErrorOrSuccessOnNotFound();
    }
    if (result == ERROR_SUCCESS)
      result = this_result;
  }
  return result;
  #endif
  return 0;
}

DWORD DoDeleteFile(const Path& path, bool recursive) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (path.empty())
    return ERROR_SUCCESS;

  if (path.path().length() >= MAX_PATH)
    return ERROR_BAD_PATHNAME;

  // Report success if the file or path does not exist.
  const DWORD attr = ::GetFileAttributes(path.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return ReturnLastErrorOrSuccessOnNotFound();

  // Clear the read-only bit if it is set.
  if ((attr & FILE_ATTRIBUTE_READONLY) &&
      !::SetFileAttributes(path.c_str(),
                           attr & ~DWORD{FILE_ATTRIBUTE_READONLY})) {
    // It's possible for |path| to be gone now under a race with other deleters.
    return ReturnLastErrorOrSuccessOnNotFound();
  }

  // Perform a simple delete on anything that isn't a directory.
  if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
    return ::DeleteFile(path.c_str()) ? ERROR_SUCCESS
                                              : ReturnLastErrorOrSuccessOnNotFound();
  }

  if (recursive) {
    const DWORD error_code = DeleteFileRecursive(path, BASE_PATH_LITERAL("*"), true);
    DCHECK(static_cast<LONG>(error_code) != ERROR_FILE_NOT_FOUND);
    DCHECK(static_cast<LONG>(error_code) != ERROR_PATH_NOT_FOUND);
    if (error_code != ERROR_SUCCESS)
      return error_code;
  }
  return ::RemoveDirectory(path.c_str())
             ? ERROR_SUCCESS
             : ReturnLastErrorOrSuccessOnNotFound();
}

bool DeleteFileOrSetLastError(const Path& path, bool recursive) {
  const DWORD error = DoDeleteFile(path, recursive);
  if (error == ERROR_SUCCESS)
    return true;

  ::SetLastError(error);
  return false;
}


bool DeletePathRecursively(const Path& path) {
  return DeleteFileOrSetLastError(path, /*recursive=*/true);
}

bool CreateDirectory(const Path& full_path) {
  const wchar_t* const full_path_str = full_path.c_str();
  const DWORD fileattr = ::GetFileAttributesW(full_path_str);
  if (fileattr != INVALID_FILE_ATTRIBUTES) {
    if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      return true;
    }
    LOG_WARNING("CreateDirectory(), conflicts with existing file.");
    ::SetLastError(ERROR_FILE_EXISTS);
    return false;
  }

  if (::CreateDirectoryW(full_path_str, NULL))
    return true;

  const DWORD error_code = ::GetLastError();
  if (error_code == ERROR_ALREADY_EXISTS && base::DirectoryExists(full_path)) {
    // This error code ERROR_ALREADY_EXISTS doesn't indicate whether we were
    // racing with someone creating the same directory, or a file with the same
    // path.  If DirectoryExists() returns true, we lost the race to create the
    // same directory.
    return true;
  }

  ::SetLastError(error_code);
  LOG_WARNING("Failed to create directory");
  return false;
}

// rather bad way of doing this..
bool GetTempDir(Path* path) {
  wchar_t temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPathW(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
    return false;
  const base::StringRefW ref(temp_path);
  *path = Path(ref);
  return true;
}

bool CreateNewTempDirectory(const Path::BufferType& prefix, Path* new_temp_path) {
  Path system_temp_dir;
  if (!GetTempDir(&system_temp_dir))
    return false;

  return base::CreateTemporaryDirInDir(system_temp_dir, prefix, new_temp_path);
}
}  // namespace base

// Copyright(C) 2021 Force67<github.com / Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/filesystem/path.h"
#include "base/filesystem/file_util.h"

#include "base/random.h"
#include <windows.h>

namespace base {

bool CreateTemporaryDirInDir(const Path& base_dir,
                             const Path::BufferType& prefix,
                             Path* new_dir) {
  Path path_to_create;

  for (int count = 0; count < 50; ++count) {
    const auto proc_id = std::to_wstring(::GetCurrentProcessId());
    const auto rand = std::to_wstring(RandomInt(INT_MIN, INT_MAX));

    path_to_create = base_dir / prefix / proc_id / "_" / rand;
    if (::CreateDirectory(path_to_create.c_str(), NULL)) {
      *new_dir = path_to_create;
      return true;
    }
  }

  return false;
}

bool DirectoryExists(const Path& path) {
  DWORD fileattr = GetFileAttributes(path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

bool CreateDirectory(const Path& full_path) {
  const wchar_t* const full_path_str = full_path.c_str();
  const DWORD fileattr = ::GetFileAttributes(full_path_str);
  if (fileattr != INVALID_FILE_ATTRIBUTES) {
    if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      return true;
    }
    LOG_WARNING("CreateDirectory(), conflicts with existing file.");
    ::SetLastError(ERROR_FILE_EXISTS);
    return false;
  }

  if (::CreateDirectory(full_path_str, NULL))
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

bool GetTempDir(Path* path) {
  wchar_t temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPath(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
    return false;
  return true;
}

bool CreateNewTempDirectory(const Path::BufferType& prefix,
                            Path* new_temp_path) {
  Path system_temp_dir;
  if (!GetTempDir(&system_temp_dir))
    return false;

  return base::CreateTemporaryDirInDir(system_temp_dir, prefix, new_temp_path);
}
}  // namespace base

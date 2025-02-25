// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "base/compiler.h"
#include "build/build_config.h"
#include "base/filesystem/file.h"

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <errno.h>
#endif

namespace base {

File::Info::Info() = default;

File::Info::~Info() = default;

File::File() = default;

#if !defined(OS_NACL)
File::File(const Path& path, uint32_t flags) : error_details_(FILE_OK) {
  Initialize(path, flags);
}
#endif

File::File(ScopedPlatformFile platform_file) : File(std::move(platform_file), false) {}

File::File(PlatformFile platform_file) : File(platform_file, false) {}

File::File(ScopedPlatformFile platform_file, bool async)
    : file_(std::move(platform_file)), error_details_(FILE_OK), async_(async) {
#if defined(OS_POSIX) || defined(OS_FUCHSIA)
  DCHECK(file_.get() >= -1);
#endif
}

File::File(PlatformFile platform_file, bool async)
    : file_(platform_file), error_details_(FILE_OK), async_(async) {
#if defined(OS_POSIX) || defined(OS_FUCHSIA)
  DCHECK(platform_file >= -1);
#endif
}

File::File(Error error_details) : error_details_(error_details) {}

File::File(File&& other)
    : file_(other.TakePlatformFile()),
      path_(other.path_),
      error_details_(other.error_details()),
      created_(other.created()),
      async_(other.async_) {}

File::~File() {
  // Go through the AssertIOAllowed logic.
  Close();
}

File& File::operator=(File&& other) noexcept {
  Close();
  SetPlatformFile(other.TakePlatformFile());
  path_ = other.path_;
  error_details_ = other.error_details();
  created_ = other.created();
  async_ = other.async_;
  return *this;
}

#if !defined(OS_NACL)
void File::Initialize(const Path& path, uint32_t flags) {
  path_ = path;
  DoInitialize(path, flags);
}
#endif

bool File::ReadAndCheck(int64_t offset, std::span<uint8_t> data) {
  int size = static_cast<int>(data.size());
  return Read(offset, reinterpret_cast<char*>(data.data()), size) == size;
}

bool File::ReadAtCurrentPosAndCheck(std::span<uint8_t> data) {
  int size = static_cast<int>(data.size());
  return ReadAtCurrentPos(reinterpret_cast<char*>(data.data()), size) == size;
}

bool File::WriteAndCheck(int64_t offset, std::span<const uint8_t> data) {
  int size = static_cast<int>(data.size());
  return Write(offset, reinterpret_cast<const char*>(data.data()), size) == size;
}

bool File::WriteAtCurrentPosAndCheck(std::span<const uint8_t> data) {
  int size = static_cast<int>(data.size());
  return WriteAtCurrentPos(reinterpret_cast<const char*>(data.data()), size) == size;
}

// static
std::string File::ErrorToString(Error error) {
  switch (error) {
    case FILE_OK:
      return "FILE_OK";
    case FILE_ERROR_FAILED:
      return "FILE_ERROR_FAILED";
    case FILE_ERROR_IN_USE:
      return "FILE_ERROR_IN_USE";
    case FILE_ERROR_EXISTS:
      return "FILE_ERROR_EXISTS";
    case FILE_ERROR_NOT_FOUND:
      return "FILE_ERROR_NOT_FOUND";
    case FILE_ERROR_ACCESS_DENIED:
      return "FILE_ERROR_ACCESS_DENIED";
    case FILE_ERROR_TOO_MANY_OPENED:
      return "FILE_ERROR_TOO_MANY_OPENED";
    case FILE_ERROR_NO_MEMORY:
      return "FILE_ERROR_NO_MEMORY";
    case FILE_ERROR_NO_SPACE:
      return "FILE_ERROR_NO_SPACE";
    case FILE_ERROR_NOT_A_DIRECTORY:
      return "FILE_ERROR_NOT_A_DIRECTORY";
    case FILE_ERROR_INVALID_OPERATION:
      return "FILE_ERROR_INVALID_OPERATION";
    case FILE_ERROR_SECURITY:
      return "FILE_ERROR_SECURITY";
    case FILE_ERROR_ABORT:
      return "FILE_ERROR_ABORT";
    case FILE_ERROR_NOT_A_FILE:
      return "FILE_ERROR_NOT_A_FILE";
    case FILE_ERROR_NOT_EMPTY:
      return "FILE_ERROR_NOT_EMPTY";
    case FILE_ERROR_INVALID_URL:
      return "FILE_ERROR_INVALID_URL";
    case FILE_ERROR_IO:
      return "FILE_ERROR_IO";
    case FILE_ERROR_MAX:
      break;
  }

  IMPOSSIBLE;
  return "";
}

base::UniquePointer<byte[]> ReadFile(const base::Path& path, i64* opt_size) {
  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid()) {
    return nullptr;
  }

  i64 length = file.Seek(base::File::Whence::FROM_END, 0);
  if (opt_size)
    *opt_size = length;

  auto content = base::MakeUnique<u8[]>(length);
  file.Seek(base::File::Whence::FROM_BEGIN, 0);
  i64 read =
      file.Read(0, reinterpret_cast<char*>(content.Get_UseOnlyIfYouKnowWhatYouareDoing()),
                static_cast<i32>(length));
  if (read == length) {
    return content;
  }

  return nullptr;
}

bool WriteFile(const base::Path& path, const byte* data, i64 size) {
  base::File file(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  if (!file.IsValid()) {
    return false;
  }

  int r = file.Write(0, reinterpret_cast<const char*>(data), static_cast<int>(size));
  if (r != size) {
    return false;
  }

  return true;
}
}  // namespace base

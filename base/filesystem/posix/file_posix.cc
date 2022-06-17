// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <base/filesystem/file.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include <base/check.h>

#include <base/filesystem/file.h>
#include <base/filesystem/posix/eintr_wrapper.h>
#include <base/text/code_convert.h>
#include <base/threading/scoped_blocking_call.h>
#include "build/build_config.h"

namespace base {
// Make sure our Whence mappings match the system headers.
static_assert(File::FROM_BEGIN == SEEK_SET && File::FROM_CURRENT == SEEK_CUR &&
                  File::FROM_END == SEEK_END,
              "whence mapping must match the system headers");

namespace {

bool IsOpenAppend(PlatformFile file) {
  return (fcntl(file, F_GETFL) & O_APPEND) != 0;
}

int CallFtruncate(PlatformFile file, int64_t length) {
  return HANDLE_EINTR(ftruncate64(file, length));
}

int CallFutimes(PlatformFile file, const struct timeval times[2]) {
#ifdef __USE_XOPEN2K8
  // futimens should be available, but futimes might not be
  // http://pubs.opengroup.org/onlinepubs/9699919799/

  timespec ts_times[2];
  ts_times[0].tv_sec = times[0].tv_sec;
  ts_times[0].tv_nsec = times[0].tv_usec * 1000;
  ts_times[1].tv_sec = times[1].tv_sec;
  ts_times[1].tv_nsec = times[1].tv_usec * 1000;

  return futimens(file, ts_times);
#else
  return futimes(file, times);
#endif
}

i16 FcntlFlockType(File::LockMode* mode) {
  if (!mode)
    return F_UNLCK;
  switch (*mode) {
    case File::LockMode::kShared:
      return F_RDLCK;
    case File::LockMode::kExclusive:
      return F_WRLCK;
  }
  IMPOSSIBLE;
  return 0;
}

File::Error CallFcntlFlock(PlatformFile file, File::LockMode* mode) {
  struct flock lock;
  lock.l_type = FcntlFlockType(mode);
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  if (HANDLE_EINTR(fcntl(file, F_SETLK, &lock)) == -1)
    return File::GetLastFileError();
  return File::FILE_OK;
}

}  // namespace

void File::Info::FromStat(const stat_wrapper_t& stat_info) {
  IMPOSSIBLE;
}

bool File::IsValid() const {
  return file_.is_valid();
}

PlatformFile File::GetPlatformFile() const {
  return file_.get();
}

PlatformFile File::TakePlatformFile() {
  return file_.release();
}

void File::Close() {
  if (!IsValid())
    return;

  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  file_.reset();
}

int64_t File::Seek(Whence whence, int64_t offset) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());

  static_assert(sizeof(int64_t) == sizeof(off_t), "off_t must be 64 bits");
  return lseek(file_.get(), static_cast<off_t>(offset), static_cast<int>(whence));
}

int File::Read(int64_t offset, char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(pread(file_.get(), data + bytes_read, size - bytes_read,
                            offset + bytes_read));
    if (rv <= 0)
      break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadAtCurrentPos(char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(read(file_.get(), data + bytes_read, size - bytes_read));
    if (rv <= 0)
      break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadNoBestEffort(int64_t offset, char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());

  return HANDLE_EINTR(pread(file_.get(), data, size, offset));
}

int File::ReadAtCurrentPosNoBestEffort(char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  return HANDLE_EINTR(read(file_.get(), data, size));
}

int File::Write(int64_t offset, const char* data, size_t size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (IsOpenAppend(file_.get()))
    return WriteAtCurrentPos(data, size);

  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_written = 0;
  int rv;
  do {
    rv = HANDLE_EINTR(pwrite(file_.get(), data + bytes_written, size - bytes_written,
                             offset + bytes_written));
    if (rv <= 0)
      break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPos(const char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  int bytes_written = 0;
  int rv;
  do {
    rv =
        HANDLE_EINTR(write(file_.get(), data + bytes_written, size - bytes_written));
    if (rv <= 0)
      break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPosNoBestEffort(const char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());
  if (size < 0)
    return -1;

  return HANDLE_EINTR(write(file_.get(), data, size));
}

int64_t File::GetLength() {
  DCHECK(IsValid());

  stat_wrapper_t file_info;
  if (Fstat(file_.get(), &file_info))
    return -1;

  return file_info.st_size;
}

bool File::SetLength(int64_t length) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());

  return !CallFtruncate(file_.get(), length);
}

#if 0
bool File::SetTimes(Time last_access_time, Time last_modified_time) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());

  timeval times[2];
  times[0] = last_access_time.ToTimeVal();
  times[1] = last_modified_time.ToTimeVal();

  return !CallFutimes(file_.get(), times);
}
#endif

bool File::GetInfo(Info* info) {
  DCHECK(IsValid());

  stat_wrapper_t file_info;
  if (Fstat(file_.get(), &file_info))
    return false;

  info->FromStat(file_info);
  return true;
}

File::Error File::Lock(File::LockMode mode) {
  return CallFcntlFlock(file_.get(), &mode);
}

File::Error File::Unlock() {
  return CallFcntlFlock(file_.get(), nullptr);
}

File File::Duplicate() const {
  if (!IsValid())
    return File();

  ScopedPlatformFile other_fd(HANDLE_EINTR(dup(GetPlatformFile())));
  if (!other_fd.is_valid())
    return File(File::GetLastFileError());

  return File(std::move(other_fd), async());
}

// Static.
File::Error File::OSErrorToFileError(int saved_errno) {
  switch (saved_errno) {
    case EACCES:
    case EISDIR:
    case EROFS:
    case EPERM:
      return FILE_ERROR_ACCESS_DENIED;
    case EBUSY:
    case ETXTBSY:
      return FILE_ERROR_IN_USE;
    case EEXIST:
      return FILE_ERROR_EXISTS;
    case EIO:
      return FILE_ERROR_IO;
    case ENOENT:
      return FILE_ERROR_NOT_FOUND;
    case ENFILE:  // fallthrough
    case EMFILE:
      return FILE_ERROR_TOO_MANY_OPENED;
    case ENOMEM:
      return FILE_ERROR_NO_MEMORY;
    case ENOSPC:
      return FILE_ERROR_NO_SPACE;
    case ENOTDIR:
      return FILE_ERROR_NOT_A_DIRECTORY;
    default:
      // This function should only be called for errors.
      DCHECK(0 != saved_errno);
      return FILE_ERROR_FAILED;
  }
}

// TODO(erikkay): does it make sense to support FLAG_EXCLUSIVE_* here?
void File::DoInitialize(const Path& path, uint32_t flags) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(!IsValid());

  int open_flags = 0;
  if (flags & FLAG_CREATE)
    open_flags = O_CREAT | O_EXCL;

  created_ = false;

  if (flags & FLAG_CREATE_ALWAYS) {
    DCHECK(!open_flags);
    DCHECK(flags & FLAG_WRITE);
    open_flags = O_CREAT | O_TRUNC;
  }

  if (flags & FLAG_OPEN_TRUNCATED) {
    DCHECK(!open_flags);
    DCHECK(flags & FLAG_WRITE);
    open_flags = O_TRUNC;
  }

  if (!open_flags && !(flags & FLAG_OPEN) && !(flags & FLAG_OPEN_ALWAYS)) {
    IMPOSSIBLE;
    errno = EOPNOTSUPP;
    error_details_ = FILE_ERROR_FAILED;
    return;
  }

  if (flags & FLAG_WRITE && flags & FLAG_READ) {
    open_flags |= O_RDWR;
  } else if (flags & FLAG_WRITE) {
    open_flags |= O_WRONLY;
  } else if (!(flags & FLAG_READ) && !(flags & FLAG_WRITE_ATTRIBUTES) &&
             !(flags & FLAG_APPEND) && !(flags & FLAG_OPEN_ALWAYS)) {
    // Note: For FLAG_WRITE_ATTRIBUTES and no other read/write flags, we'll
    // open the file in O_RDONLY mode (== 0, see static_assert below), so that
    // we get a fd that can be used for SetTimes().
    IMPOSSIBLE;
  }

  if (flags & FLAG_TERMINAL_DEVICE)
    open_flags |= O_NOCTTY | O_NDELAY;

  if (flags & FLAG_APPEND && flags & FLAG_READ)
    open_flags |= O_APPEND | O_RDWR;
  else if (flags & FLAG_APPEND)
    open_flags |= O_APPEND | O_WRONLY;

  static_assert(O_RDONLY == 0, "O_RDONLY must equal zero");

  int mode = S_IRUSR | S_IWUSR;

  int descriptor = HANDLE_EINTR(open(path.c_str(), open_flags, mode));

  if (flags & FLAG_OPEN_ALWAYS) {
    if (descriptor < 0) {
      open_flags |= O_CREAT;
      descriptor = HANDLE_EINTR(open(path.c_str(), open_flags, mode));
      if (descriptor >= 0)
        created_ = true;
    }
  }

  if (descriptor < 0) {
    error_details_ = File::GetLastFileError();
    return;
  }

  if (flags & (FLAG_CREATE_ALWAYS | FLAG_CREATE))
    created_ = true;

  if (flags & FLAG_DELETE_ON_CLOSE)
    unlink(path.c_str());

  async_ = ((flags & FLAG_ASYNC) == FLAG_ASYNC);
  error_details_ = FILE_OK;
  file_.reset(descriptor);
}

bool File::Flush() {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK(IsValid());

  return !HANDLE_EINTR(fdatasync(file_.get()));
}

void File::SetPlatformFile(PlatformFile file) {
  DCHECK(!file_.is_valid());
  file_.reset(file);
}

// static
File::Error File::GetLastFileError() {
  return base::File::OSErrorToFileError(errno);
}

int File::Stat(const char* path, stat_wrapper_t* sb) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return stat64(path, sb);
}
int File::Fstat(int fd, stat_wrapper_t* sb) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return fstat64(fd, sb);
}
int File::Lstat(const char* path, stat_wrapper_t* sb) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return lstat64(path, sb);
}
}  // namespace base

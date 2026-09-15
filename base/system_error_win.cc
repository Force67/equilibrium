// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/system_error.h>

#include <errno.h>

#include <base/strings/format.h>

namespace base {

const char* ErrnoName(int error) noexcept {
  switch (error) {
#define BASE_ERRNO_CASE(NAME) \
  case NAME:                  \
    return #NAME;
    BASE_ERRNO_CASE(EPERM)
    BASE_ERRNO_CASE(ENOENT)
    BASE_ERRNO_CASE(ESRCH)
    BASE_ERRNO_CASE(EINTR)
    BASE_ERRNO_CASE(EIO)
    BASE_ERRNO_CASE(ENXIO)
    BASE_ERRNO_CASE(E2BIG)
    BASE_ERRNO_CASE(ENOEXEC)
    BASE_ERRNO_CASE(EBADF)
    BASE_ERRNO_CASE(ECHILD)
    BASE_ERRNO_CASE(EAGAIN)
    BASE_ERRNO_CASE(ENOMEM)
    BASE_ERRNO_CASE(EACCES)
    BASE_ERRNO_CASE(EFAULT)
    BASE_ERRNO_CASE(EBUSY)
    BASE_ERRNO_CASE(EEXIST)
    BASE_ERRNO_CASE(EXDEV)
    BASE_ERRNO_CASE(ENODEV)
    BASE_ERRNO_CASE(ENOTDIR)
    BASE_ERRNO_CASE(EISDIR)
    BASE_ERRNO_CASE(EINVAL)
    BASE_ERRNO_CASE(ENFILE)
    BASE_ERRNO_CASE(EMFILE)
    BASE_ERRNO_CASE(ENOTTY)
    BASE_ERRNO_CASE(EFBIG)
    BASE_ERRNO_CASE(ENOSPC)
    BASE_ERRNO_CASE(ESPIPE)
    BASE_ERRNO_CASE(EROFS)
    BASE_ERRNO_CASE(EMLINK)
    BASE_ERRNO_CASE(EPIPE)
    BASE_ERRNO_CASE(EDOM)
    BASE_ERRNO_CASE(ERANGE)
    BASE_ERRNO_CASE(EDEADLK)
    BASE_ERRNO_CASE(ENAMETOOLONG)
    BASE_ERRNO_CASE(ENOLCK)
    BASE_ERRNO_CASE(ENOSYS)
    BASE_ERRNO_CASE(ENOTEMPTY)
    BASE_ERRNO_CASE(EILSEQ)
#undef BASE_ERRNO_CASE
    default:
      break;
  }
  static thread_local char unknown[16];
  const mem_size length = FormatTo(unknown, sizeof(unknown), "E{}", error);
  if (length >= sizeof(unknown))
    return "E?";
  return unknown;
}

}  // namespace base

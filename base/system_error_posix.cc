// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/system_error.h>

#include <errno.h>

#include <base/strings/format.h>

namespace base {

const char* ErrnoName(int error) noexcept {
  switch (error) {
// The optional names are wrapped: EBADFD is Linux-only, and the socket
// errors are absent on some targets. A name the platform does not define
// simply drops out of the table instead of failing the build.
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
#ifdef ETXTBSY
    BASE_ERRNO_CASE(ETXTBSY)
#endif
    BASE_ERRNO_CASE(EFBIG)
    BASE_ERRNO_CASE(ENOSPC)
    BASE_ERRNO_CASE(ESPIPE)
    BASE_ERRNO_CASE(EROFS)
    BASE_ERRNO_CASE(EMLINK)
    BASE_ERRNO_CASE(EPIPE)
    BASE_ERRNO_CASE(EDOM)
    BASE_ERRNO_CASE(ERANGE)
#ifdef EDEADLK
    BASE_ERRNO_CASE(EDEADLK)
#endif
#ifdef ENAMETOOLONG
    BASE_ERRNO_CASE(ENAMETOOLONG)
#endif
#ifdef ENOLCK
    BASE_ERRNO_CASE(ENOLCK)
#endif
#ifdef ENOSYS
    BASE_ERRNO_CASE(ENOSYS)
#endif
#ifdef ENOTEMPTY
    BASE_ERRNO_CASE(ENOTEMPTY)
#endif
#ifdef ELOOP
    BASE_ERRNO_CASE(ELOOP)
#endif
#ifdef ENOMSG
    BASE_ERRNO_CASE(ENOMSG)
#endif
#ifdef EOVERFLOW
    BASE_ERRNO_CASE(EOVERFLOW)
#endif
#ifdef EBADFD
    BASE_ERRNO_CASE(EBADFD)
#endif
#ifdef ENOTSOCK
    BASE_ERRNO_CASE(ENOTSOCK)
#endif
#ifdef EADDRINUSE
    BASE_ERRNO_CASE(EADDRINUSE)
#endif
#ifdef EADDRNOTAVAIL
    BASE_ERRNO_CASE(EADDRNOTAVAIL)
#endif
#ifdef ENETDOWN
    BASE_ERRNO_CASE(ENETDOWN)
#endif
#ifdef ENETUNREACH
    BASE_ERRNO_CASE(ENETUNREACH)
#endif
#ifdef ECONNABORTED
    BASE_ERRNO_CASE(ECONNABORTED)
#endif
#ifdef ECONNRESET
    BASE_ERRNO_CASE(ECONNRESET)
#endif
#ifdef ENOBUFS
    BASE_ERRNO_CASE(ENOBUFS)
#endif
#ifdef ETIMEDOUT
    BASE_ERRNO_CASE(ETIMEDOUT)
#endif
#ifdef ECONNREFUSED
    BASE_ERRNO_CASE(ECONNREFUSED)
#endif
#ifdef EHOSTDOWN
    BASE_ERRNO_CASE(EHOSTDOWN)
#endif
#ifdef EHOSTUNREACH
    BASE_ERRNO_CASE(EHOSTUNREACH)
#endif
#ifdef ECANCELED
    BASE_ERRNO_CASE(ECANCELED)
#endif
#undef BASE_ERRNO_CASE
    default:
      break;
  }
  // EWOULDBLOCK and EDEADLOCK alias other values on Linux, so they are not
  // listed above; anything else is rendered numerically.
  static thread_local char unknown[16];
  const mem_size length = FormatTo(unknown, sizeof(unknown), "E{}", error);
  if (length >= sizeof(unknown))
    return "E?";
  return unknown;
}

}  // namespace base

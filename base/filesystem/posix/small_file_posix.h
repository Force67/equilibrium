// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Reading a small file whole, without stdio.
//
// The callers are all reading the virtual files under /proc, which is also
// why base::File is not the right tool: those report a size of zero, so they
// have to be read until EOF rather than sized and then read, and they are
// small enough that one buffer always suffices.
#pragma once

#include <base/arch.h>
#include <base/filesystem/posix/eintr_wrapper.h>

#include <fcntl.h>
#include <unistd.h>

namespace base {

// Reads at most |capacity| - 1 bytes of |path| into |buffer| and terminates
// it. Returns the number of bytes read, or -1 if the file could not be
// opened. A file longer than the buffer is truncated, not an error.
inline i64 ReadSmallFile(const char* path, char* buffer,
                         mem_size capacity) noexcept {
  if (capacity == 0)
    return -1;
  const int descriptor = HANDLE_EINTR(::open(path, O_RDONLY | O_CLOEXEC));
  if (descriptor < 0)
    return -1;

  mem_size total = 0;
  while (total + 1 < capacity) {
    const ssize_t chunk =
        HANDLE_EINTR(::read(descriptor, buffer + total, capacity - 1 - total));
    if (chunk <= 0)
      break;  // End of file, or an error with nothing more to be done about it.
    total += static_cast<mem_size>(chunk);
  }
  IGNORE_EINTR(::close(descriptor));

  buffer[total] = '\0';
  return static_cast<i64>(total);
}

}  // namespace base

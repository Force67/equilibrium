// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/standard_streams.h>

#include <errno.h>
#include <unistd.h>

#include <base/compiler.h>

namespace base {
namespace {

void WriteAll(int descriptor, const char* text, mem_size length) noexcept {
  mem_size written = 0;
  while (written < length) {
    const ssize_t result =
        ::write(descriptor, text + written, length - written);
    if (result > 0) {
      written += static_cast<mem_size>(result);
      continue;
    }
    // A signal can cut a write short; anything else means the stream is gone
    // and there is nowhere left to report it.
    if (result < 0 && errno == EINTR)
      continue;
    return;
  }
}

}  // namespace

void WriteStandardError(const char* text, mem_size length) noexcept {
  WriteAll(STDERR_FILENO, text, length);
}

void WriteStandardOutput(const char* text, mem_size length) noexcept {
  WriteAll(STDOUT_FILENO, text, length);
}

void TerminateAbnormally() noexcept {
  DEBUG_TRAP;
  // Reached only where the trap is compiled out. _exit is the raw syscall, so
  // it skips the atexit handlers and stream flushes abort() would run.
  ::_exit(3);
  __builtin_unreachable();
}

}  // namespace base

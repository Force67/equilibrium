// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/standard_streams.h>

#include <base/compiler.h>
#include <base/win/minwin.h>

namespace base {
namespace {

void WriteAll(DWORD stream, const char* text, mem_size length) noexcept {
  const HANDLE handle = ::GetStdHandle(stream);
  if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
    return;
  mem_size written = 0;
  while (written < length) {
    DWORD chunk = 0;
    if (!::WriteFile(handle, text + written,
                     static_cast<DWORD>(length - written), &chunk, nullptr) ||
        chunk == 0) {
      return;  // The stream is gone; there is nowhere left to report it.
    }
    written += chunk;
  }
}

}  // namespace

void WriteStandardError(const char* text, mem_size length) noexcept {
  WriteAll(STD_ERROR_HANDLE, text, length);
}

void WriteStandardOutput(const char* text, mem_size length) noexcept {
  WriteAll(STD_OUTPUT_HANDLE, text, length);
}

void TerminateAbnormally() noexcept {
  DEBUG_TRAP;
  // Reached only where the trap is compiled out. TerminateProcess skips the
  // orderly CRT shutdown that abort() would run.
  ::TerminateProcess(::GetCurrentProcess(), 3);
  __builtin_unreachable();
}

}  // namespace base

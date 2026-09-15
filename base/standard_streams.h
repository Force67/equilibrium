// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Unbuffered writes to the process's standard streams, and the abnormal exit
// a failed check takes.
//
// This exists so base's diagnostics do not go through stdio. FILE* buffering
// is the wrong behaviour for them anyway: a fatal check that writes its reason
// into a buffer and then traps loses the message, which is the one thing that
// had to survive. A write straight to the descriptor cannot.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

// Writes |length| bytes to the standard error, retrying a partial or
// interrupted write. Best effort: a closed or failing stream is not reported,
// because every caller is already in the middle of reporting something else.
BASE_EXPORT void WriteStandardError(const char* text, mem_size length) noexcept;

// As WriteStandardError, to the standard output.
BASE_EXPORT void WriteStandardOutput(const char* text, mem_size length) noexcept;

// Ends the process immediately and abnormally, without running destructors,
// atexit handlers or stream flushes. Traps first so an attached debugger stops
// at the fault rather than inside a runtime teardown path.
[[noreturn]] BASE_EXPORT void TerminateAbnormally() noexcept;

}  // namespace base

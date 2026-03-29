// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/vector.h>
#include <base/strings/xstring.h>

namespace base {

// Returns true if a debugger (gdb, lldb, etc.) is attached to this process.
// On Linux, reads /proc/self/status for TracerPid.
// On Windows, calls IsDebuggerPresent().
bool IsDebuggerAttached();

// Capture a callstack from the current point.  Returns demangled
// frame strings (function + offset).  |skipFrames| omits the top N
// frames (e.g. 1 to skip CaptureCallstack itself).
Vector<String> CaptureCallstack(i32 skipFrames = 1, i32 maxFrames = 32);

}  // namespace base

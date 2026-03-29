// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

// Returns true if a debugger (gdb, lldb, etc.) is attached to this process.
// On Linux, reads /proc/self/status for TracerPid.
// On Windows, calls IsDebuggerPresent().
bool IsDebuggerAttached();

}  // namespace base

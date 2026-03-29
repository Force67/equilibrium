// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "debugging.h"

#if defined(OS_WIN)
#include <Windows.h>
#endif

namespace base {

bool IsDebuggerAttached() {
#if defined(OS_WIN)
  return ::IsDebuggerPresent() != 0;
#else
  return false;
#endif
}

}  // namespace base

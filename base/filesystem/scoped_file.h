// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <stdio.h>

#include <memory>

#include "base/export.h"
#include "base/scoped_generic.h"
#include "build/build_config.h"

namespace base {

namespace internal {
#if defined(OS_CHROMEOS) || defined(OS_LINUX)
// On ChromeOS and Linux, FD lifetime is guarded by a global table plus a
// libc close() hook for checks.
struct BASE_EXPORT ScopedFDCloseTraits : public ScopedGenericOwnershipTracking {
#else
struct BASE_EXPORT ScopedFDCloseTraits {
#endif
  static int InvalidValue() { return -1; }
  static void Free(int fd);
#if defined(OS_CHROMEOS) || defined(OS_LINUX)
  static void Acquire(const ScopedGeneric<int, ScopedFDCloseTraits>&, int);
  static void Release(const ScopedGeneric<int, ScopedFDCloseTraits>&, int);
#endif
};
}  // namespace internal

#if defined(OS_CHROMEOS) || defined(OS_LINUX)
namespace subtle {

// Enables or disables enforcement of FD ownership as tracked by ScopedFD
// objects. Enforcement is disabled by default since it proves unwieldy in some
// test environments, but tracking is always done. It's best to enable this as
// early as possible in a process's lifetime.
void BASE_EXPORT EnableFDOwnershipEnforcement(bool enabled);

// Resets ownership state of all FDs. Only for a forked child between fork()
// and exec(): mass-closing FDs before exec requires the reset, and the state
// may be inconsistent after fork() in a multithreaded parent. Call this as
// early as possible if any FD manipulation happens before exec.
void BASE_EXPORT ResetFDOwnership();

}  // namespace subtle
#endif

// -----------------------------------------------------------------------------

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
// A low-level POSIX file descriptor closer for platform-specific code that
// does non-file things with the FD (sockets and the like). For cross-platform
// file handling use base::File; for Windows handles see base/win/scoped_handle.h.
typedef ScopedGeneric<int, internal::ScopedFDCloseTraits> ScopedFD;
#endif

#if defined(OS_CHROMEOS) || defined(OS_LINUX)
// Queries the ownership status of an FD, i.e. whether it is currently owned by
// a ScopedFD in the calling process.
bool BASE_EXPORT IsFDOwned(int fd);
#endif  // defined(OS_CHROMEOS) || defined(OS_LINUX)

}  // namespace base
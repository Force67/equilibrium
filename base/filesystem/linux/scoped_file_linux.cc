// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <base/check.h>
#include <base/filesystem/scoped_file.h>
#include "compiler.h"

namespace {

// We want to avoid any kind of allocations in our close() implementation, so we
// use a fixed-size table. Given our common FD limits and the preference for new
// FD allocations to use the lowest available descriptor, this should be
// sufficient to guard most FD lifetimes. The worst case scenario if someone
// attempts to own a higher FD is that we don't track it.
const int kMaxTrackedFds = 4096;

NOINLINE void CrashOnFdOwnershipViolation() {
  DEBUG_TRAP;
}

bool CanTrack(int fd) {
  return fd >= 0 && fd < kMaxTrackedFds;
}

void UpdateAndCheckFdOwnership(int fd, bool owned) {
#if 0
  if (CanTrack(fd) && g_is_fd_owned[fd].exchange(owned) == owned &&
      g_is_ownership_enforced) {
    CrashOnFdOwnershipViolation();
  }
#endif
  DEBUG_TRAP;
}

}  // namespace

namespace base {
namespace internal {

// static
void ScopedFDCloseTraits::Acquire(const ScopedFD& owner, int fd) {
  // UpdateAndCheckFdOwnership(fd, /*owned=*/true);
  //  TBP
}

// static
void ScopedFDCloseTraits::Release(const ScopedFD& owner, int fd) {
  // UpdateAndCheckFdOwnership(fd, /*owned=*/false);
}

}  // namespace internal

}  // namespace base

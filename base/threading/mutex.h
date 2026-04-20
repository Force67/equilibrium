// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// base::Mutex / base::SharedMutex are typedef aliases today, but callers must
// treat them as opaque types: always acquire via base::LockGuard /
// base::UniqueLock / base::SharedLockGuard (base/threading/lock_guard.h).
// This leaves the implementation free to swap out later without touching
// consumers.

#include <mutex>
#include <shared_mutex>

namespace base {
using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
}  // namespace base

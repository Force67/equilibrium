// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// SharedMutex on SRWLOCK. Out of line rather than inline in mutex.h so that
// minwin.h, which on mingw is all of <windows.h>, stays out of every file that
// merely holds a mutex: its macros (MemoryBarrier, near, far, ...) would
// otherwise rewrite unrelated names there.

#include <base/threading/mutex.h>
#include <base/win/minwin.h>

namespace base {

#if !defined(BASE_USE_STD_MUTEX) || !BASE_USE_STD_MUTEX

void SharedMutex::lock() {
  ::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

void SharedMutex::unlock() {
  ::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

bool SharedMutex::try_lock() {
  return !!::TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

void SharedMutex::lock_shared() {
  ::AcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_));
}

void SharedMutex::unlock_shared() {
  ::ReleaseSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_));
}

bool SharedMutex::try_lock_shared() {
  return !!::TryAcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_));
}

#endif

}  // namespace base

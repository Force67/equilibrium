// Copyright (C) 2022-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Mutex aliases base::SpinningMutex (futex / SRWLOCK).
// base::SharedMutex wraps pthread_rwlock_t on POSIX and SRWLOCK on Windows.
// Acquire via base::LockGuard / UniqueLock / SharedLockGuard.
//
// BASE_USE_STD_MUTEX switches both to <mutex>/<shared_mutex>. Auto on MSVC.

#pragma once

#if !defined(BASE_USE_STD_MUTEX)
#  if defined(_MSC_VER) && !defined(__clang__)
#    define BASE_USE_STD_MUTEX 1
#  endif
#endif

#if defined(BASE_USE_STD_MUTEX) && BASE_USE_STD_MUTEX

#include <mutex>
#include <shared_mutex>

namespace base {
using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
}  // namespace base

#else

#include <base/threading/spinning_mutex.h>

#if defined(__linux__) || defined(OS_LINUX) || defined(OS_POSIX)
#include <pthread.h>
#elif defined(_WIN32) || defined(OS_WIN)
#include <base/win/minwin.h>
#endif

namespace base {

using Mutex = SpinningMutex;

#if defined(__linux__) || defined(OS_LINUX) || defined(OS_POSIX)

class SharedMutex {
 public:
  SharedMutex() noexcept { ::pthread_rwlock_init(&lock_, nullptr); }
  ~SharedMutex() { ::pthread_rwlock_destroy(&lock_); }

  SharedMutex(const SharedMutex&) = delete;
  SharedMutex& operator=(const SharedMutex&) = delete;

  void lock() { ::pthread_rwlock_wrlock(&lock_); }
  void unlock() { ::pthread_rwlock_unlock(&lock_); }
  bool try_lock() { return ::pthread_rwlock_trywrlock(&lock_) == 0; }

  void lock_shared() { ::pthread_rwlock_rdlock(&lock_); }
  void unlock_shared() { ::pthread_rwlock_unlock(&lock_); }
  bool try_lock_shared() { return ::pthread_rwlock_tryrdlock(&lock_) == 0; }

 private:
  pthread_rwlock_t lock_;
};

#elif defined(_WIN32) || defined(OS_WIN)

class SharedMutex {
 public:
  constexpr SharedMutex() noexcept = default;

  SharedMutex(const SharedMutex&) = delete;
  SharedMutex& operator=(const SharedMutex&) = delete;

  void lock() { ::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_)); }
  void unlock() { ::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_)); }
  bool try_lock() {
    return !!::TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
  }

  void lock_shared() { ::AcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_)); }
  void unlock_shared() { ::ReleaseSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_)); }
  bool try_lock_shared() {
    return !!::TryAcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&lock_));
  }

 private:
  // Matches the SRWLOCK ABI (single void*). SRWLOCK_INIT is {nullptr}.
  void* lock_ = nullptr;
};

#else
#error "base::SharedMutex: no native implementation for this platform"
#endif

}  // namespace base

#endif  // BASE_USE_STD_MUTEX

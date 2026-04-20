// Copyright (C) 2022-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Mutex / base::SharedMutex — STL-free mutex primitives.
//
// Mutex is backed by base::SpinningMutex (futex on Linux, SRWLOCK on
// Windows). The short spin before the blocking wait matches what std::mutex
// does on Linux anyway, at a fraction of the include cost.
//
// SharedMutex uses pthread_rwlock_t on POSIX and SRWLOCK on Windows — both
// already solve the reader/writer coordination correctly; there's no reason
// to roll our own.
//
// Fallback: define BASE_USE_STD_MUTEX to alias both to the <mutex> and
// <shared_mutex> versions instead. Auto-selected on MSVC (where we'd
// otherwise need a separate native path).
//
// Always acquire via base::LockGuard / base::UniqueLock / base::SharedLockGuard
// (base/threading/lock_guard.h). The underlying type is an implementation
// detail — callers must only rely on the lock()/unlock()/try_lock() shape.

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

// Windows SRWLOCK serves both readers and writers; different API entry
// points pick the mode. SRWLOCK_INIT is a zero-initialised struct so the
// default constructor doesn't need to run any code.
struct PA_CHROME_SRWLOCK_BARE {
  void* Ptr;
};

extern "C" __declspec(dllimport) void __stdcall AcquireSRWLockExclusive(
    PA_CHROME_SRWLOCK_BARE*);
extern "C" __declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(
    PA_CHROME_SRWLOCK_BARE*);
extern "C" __declspec(dllimport) int __stdcall TryAcquireSRWLockExclusive(
    PA_CHROME_SRWLOCK_BARE*);
extern "C" __declspec(dllimport) void __stdcall AcquireSRWLockShared(
    PA_CHROME_SRWLOCK_BARE*);
extern "C" __declspec(dllimport) void __stdcall ReleaseSRWLockShared(
    PA_CHROME_SRWLOCK_BARE*);
extern "C" __declspec(dllimport) int __stdcall TryAcquireSRWLockShared(
    PA_CHROME_SRWLOCK_BARE*);

class SharedMutex {
 public:
  constexpr SharedMutex() noexcept = default;

  SharedMutex(const SharedMutex&) = delete;
  SharedMutex& operator=(const SharedMutex&) = delete;

  void lock() { AcquireSRWLockExclusive(&lock_); }
  void unlock() { ReleaseSRWLockExclusive(&lock_); }
  bool try_lock() { return TryAcquireSRWLockExclusive(&lock_) != 0; }

  void lock_shared() { AcquireSRWLockShared(&lock_); }
  void unlock_shared() { ReleaseSRWLockShared(&lock_); }
  bool try_lock_shared() { return TryAcquireSRWLockShared(&lock_) != 0; }

 private:
  PA_CHROME_SRWLOCK_BARE lock_{nullptr};  // SRWLOCK_INIT
};

#else
#error "base::SharedMutex: no native implementation for this platform"
#endif

}  // namespace base

#endif  // BASE_USE_STD_MUTEX

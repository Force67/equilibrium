// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Inspired by:
// source.chromium.org/chromium/chromium/src/+/main:base/allocator/partition_allocator/spinning_mutex.h?q=spinlock&ss=chromium%2Fchromium%2Fsrc&start=21

#include <base/compiler.h>
#include "build/build_config.h"

#if defined(OS_POSIX)
#include <errno.h>
#include <pthread.h>
#include <base/atomic.h>
#endif

#if defined(OS_WIN)
#include <base/win/minwin.h>
#endif

namespace base {

// The behavior of this class depends on whether PA_HAS_FAST_MUTEX is defined.
// 1. When it is defined:
//
// Simple spinning lock. It will spin in user space a set number of times before
// going into the kernel to sleep.
//
// This is intended to give "the best of both worlds" between a SpinLock and
// base::Lock:
// - SpinLock: Inlined fast path, no external function calls, just
//   compare-and-swap. Short waits do not go into the kernel. Good behavior in
//   low contention cases.
// - base::Lock: Good behavior in case of contention.
//
// We don't rely on base::Lock which we could make spin (by calling Try() in a
// loop), as performance is below a custom spinlock as seen on high-level
// benchmarks. Instead this implements a simple non-recursive mutex on top of
// the futex() syscall on Linux, SRWLock on Windows, os_unfair_lock on macOS,
// and pthread_mutex on POSIX. The main difference between this and a libc
// implementation is that it only supports the simplest path: private (to a
// process), non-recursive mutexes with no priority inheritance, no timed waits.
//
// As an interesting side-effect to be used in the allocator, this code does not
// make any allocations, locks are small with a constexpr constructor and no
// destructor.
//
// 2. Otherwise: This is a simple SpinLock, in the sense that it does not have
// any awareness of other threads' behavior.
class SpinningMutex {
 public:
  inline constexpr SpinningMutex();
  STRONG_INLINE void Acquire();
  STRONG_INLINE void Release();
  STRONG_INLINE bool Try();
  void AssertAcquired() const {}  // Not supported.
  void Reinit();

 private:
  NOINLINE void AcquireSpinThenBlock();
  void LockSlow();

  // See below, the latency of PA_YIELD_PROCESSOR can be as high as ~150
  // cycles. Meanwhile, sleeping costs a few us. Spinning 64 times at 3GHz would
  // cost 150 * 64 / 3e9 ~= 3.2us.
  //
  // This applies to Linux kernels, on x86_64. On ARM we might want to spin
  // more.
  static constexpr int kSpinCount = 64;

#if defined(OS_LINUX)
  void FutexWait();
  void FutexWake();

  static constexpr int kUnlocked = 0;
  static constexpr int kLockedUncontended = 1;
  static constexpr int kLockedContended = 2;

  base::Atomic<int32_t> state_{kUnlocked};
#elif defined(OS_WIN)
  struct PA_CHROME_SRWLOCK {
    void* Ptr;
  };
  PA_CHROME_SRWLOCK lock_{SRWLOCK_INIT};
#endif
};

STRONG_INLINE void SpinningMutex::Acquire() {
  // Not marked PA_LIKELY(), as:
  // 1. We don't know how much contention the lock would experience
  // 2. This may lead to weird-looking code layout when inlined into a caller
  // with (UN)PA_LIKELY() annotations.
  if (Try())
    return;

  return AcquireSpinThenBlock();
}

inline constexpr SpinningMutex::SpinningMutex() = default;

#if defined(OS_LINUX)

STRONG_INLINE bool SpinningMutex::Try() {
  // Using the weak variant of compare_exchange(), which may fail spuriously. On
  // some architectures such as ARM, CAS is typically performed as a LDREX/STREX
  // pair, where the store may fail. In the strong version, there is a loop
  // inserted by the compiler to retry in these cases.
  //
  // Since we are retrying in Lock() anyway, there is no point having two nested
  // loops.
  int expected = kUnlocked;
  return (state_.load(std::memory_order_relaxed) == expected) &&
         state_.compare_exchange_weak(expected, kLockedUncontended,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed);
}

STRONG_INLINE void SpinningMutex::Release() {
  if ((state_.exchange(kUnlocked, std::memory_order_release) ==
       kLockedContended)) {  // likelz
    // |kLockedContended|: there is a waiter to wake up.
    //
    // Here there is a window where the lock is unlocked, since we just set it
    // to |kUnlocked| above. Meaning that another thread can grab the lock
    // in-between now and |FutexWake()| waking up a waiter. Aside from
    // potentially fairness, this is not an issue, as the newly-awaken thread
    // will check that the lock is still free.
    //
    // There is a small pessimization here though: if we have a single waiter,
    // then when it wakes up, the lock will be set to |kLockedContended|, so
    // when this waiter releases the lock, it will needlessly call
    // |FutexWake()|, even though there are no waiters. This is supported by the
    // kernel, and is what bionic (Android's libc) also does.
    FutexWake();
  }
}
#endif

#if defined(OS_WIN)

STRONG_INLINE void SpinningMutex::LockSlow() {
  ::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

STRONG_INLINE bool SpinningMutex::Try() {
  return !!::TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

STRONG_INLINE void SpinningMutex::Release() {
  ::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}
#endif

}  // namespace base

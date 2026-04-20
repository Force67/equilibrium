// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/export.h>

#include <cstdint>

#if defined(__linux__) || defined(OS_LINUX)
#include <errno.h>
#include <pthread.h>
#include <base/atomic.h>
#elif defined(_WIN32) || defined(OS_WIN)
#include <base/win/minwin.h>
#endif

namespace base {

class BASE_EXPORT SpinningMutex {
 public:
  inline constexpr SpinningMutex();
  STRONG_INLINE void Acquire();
  STRONG_INLINE void Release();
  STRONG_INLINE bool Try();
  void AssertAcquired() const {}
  void Reinit();

  // std::mutex-compatible aliases so the same type can back a
  // base::LockGuard / base::UniqueLock without a wrapper.
  STRONG_INLINE void lock() { Acquire(); }
  STRONG_INLINE void unlock() { Release(); }
  STRONG_INLINE bool try_lock() { return Try(); }

 private:
  NOINLINE void AcquireSpinThenBlock();
  void LockSlow();

  static constexpr int kSpinCount = 64;

#if defined(__linux__) || defined(OS_LINUX)
  void FutexWait();
  void FutexWake();

  static constexpr int kUnlocked = 0;
  static constexpr int kLockedUncontended = 1;
  static constexpr int kLockedContended = 2;

  base::Atomic<int32_t> state_{kUnlocked};
#elif defined(_WIN32) || defined(OS_WIN)
  struct PA_CHROME_SRWLOCK {
    void* Ptr;
  };
  PA_CHROME_SRWLOCK lock_{SRWLOCK_INIT};
#endif
};

STRONG_INLINE void SpinningMutex::Acquire() {
  if (Try())
    return;
  return AcquireSpinThenBlock();
}

inline constexpr SpinningMutex::SpinningMutex() = default;

#if defined(__linux__) || defined(OS_LINUX)

STRONG_INLINE bool SpinningMutex::Try() {
  int expected = kUnlocked;
  return (state_.load(base::memory_order_relaxed) == expected) &&
         state_.compare_exchange_weak(expected, kLockedUncontended,
                                      base::memory_order_acquire,
                                      base::memory_order_relaxed);
}

STRONG_INLINE void SpinningMutex::Release() {
  if ((state_.exchange(kUnlocked, base::memory_order_release) ==
       kLockedContended)) {
    FutexWake();
  }
}
#endif

#if defined(_WIN32) || defined(OS_WIN)

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

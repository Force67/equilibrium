// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/spinning_mutex.h>
#include <base/math/value_bounds.h>
#include <base/check.h>

#if defined(_WIN32) || defined(OS_WIN)
#include <intrin.h>
#define PA_YIELD_PROCESSOR _mm_pause()
#elif defined(__x86_64__) || defined(__i386__)
#define PA_YIELD_PROCESSOR __asm__ __volatile__("pause")
#elif defined(__aarch64__)
#define PA_YIELD_PROCESSOR __asm__ __volatile__("yield")
#else
#define PA_YIELD_PROCESSOR ((void)0)
#endif

#if defined(__linux__) || defined(OS_LINUX)
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#endif

namespace base {

void SpinningMutex::Reinit() {
  Release();
}

void SpinningMutex::AcquireSpinThenBlock() {
  int tries = 0;
  int backoff = 1;
  do {
    if (Try()) [[likely]]
      return;
    for (int yields = 0; yields < backoff; yields++) {
      PA_YIELD_PROCESSOR;
      tries++;
    }
    constexpr int kMaxBackoff = 16;
    backoff = base::Min(kMaxBackoff, backoff << 1);
  } while (tries < kSpinCount);

  LockSlow();
}

#if defined(__linux__) || defined(OS_LINUX)

void SpinningMutex::FutexWait() {
  int saved_errno = errno;
  syscall(SYS_futex, &state_, FUTEX_WAIT | FUTEX_PRIVATE_FLAG,
          kLockedContended, nullptr, nullptr, 0);
  errno = saved_errno;
}

void SpinningMutex::FutexWake() {
  int saved_errno = errno;
  syscall(SYS_futex, &state_, FUTEX_WAKE | FUTEX_PRIVATE_FLAG,
          1, nullptr, nullptr, 0);
  errno = saved_errno;
}

void SpinningMutex::LockSlow() {
  while (state_.exchange(kLockedContended, std::memory_order_acquire) != kUnlocked) {
    FutexWait();
  }
}
#endif

}  // namespace base

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
#include <cerrno>
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
  while (state_.exchange(kLockedContended, base::memory_order_acquire) != kUnlocked) {
    FutexWait();
  }
}
#endif

#if defined(__APPLE__) || defined(OS_MAC)

// macOS has no futex; __ulock is the kernel wait/wake primitive libc++ and the
// common lock implementations build on. UL_COMPARE_AND_WAIT blocks while the
// 32-bit word at the address still equals the passed value, matching the Linux
// FUTEX_WAIT contract used above; ULF_NO_ERRNO keeps it from touching errno.
extern "C" int __ulock_wait(uint32_t operation, void* addr, uint64_t value,
                            uint32_t timeout_us);
extern "C" int __ulock_wake(uint32_t operation, void* addr, uint64_t wake_value);

namespace {
constexpr uint32_t kUlCompareAndWait = 1;
constexpr uint32_t kUlfNoErrno = 0x01000000;
}  // namespace

void SpinningMutex::FutexWait() {
  __ulock_wait(kUlCompareAndWait | kUlfNoErrno, &state_, kLockedContended, 0);
}

void SpinningMutex::FutexWake() {
  __ulock_wake(kUlCompareAndWait | kUlfNoErrno, &state_, 0);
}

void SpinningMutex::LockSlow() {
  while (state_.exchange(kLockedContended, base::memory_order_acquire) != kUnlocked) {
    FutexWait();
  }
}
#endif

}  // namespace base

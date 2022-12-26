// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/spinning_mutex.h>
#include "check.h"

#if (OS_WIN)
#include <intrin.h>
#define PA_YIELD_PROCESSOR _mm_pause()
#endif

#if (OS_LINUX)
#define PA_YIELD_PROCESSOR __asm__ __volatile__("pause")
#endif

namespace {
template <class T>
const T& XX_min(const T& a, const T& b) {
  return (b < a) ? b : a;
}
}  // namespace

namespace base {

void SpinningMutex::Reinit() {
#if !defined(OS_MACOSX)
  // On most platforms, no need to re-init the lock, can just unlock it.
  Release();
#else
  unfair_lock_ = OS_UNFAIR_LOCK_INIT;
#endif  // BUILDFLAG(IS_APPLE)
}

void SpinningMutex::AcquireSpinThenBlock() {
  int tries = 0;
  int backoff = 1;
  do {
    if (Try()) [[likely]]
      return;
    // Note: Per the intel optimization manual
    // (https://software.intel.com/content/dam/develop/public/us/en/documents/64-ia-32-architectures-optimization-manual.pdf),
    // the "pause" instruction is more costly on Skylake Client than on previous
    // architectures. The latency is found to be 141 cycles
    // there (from ~10 on previous ones, nice 14x).
    //
    // According to Agner Fog's instruction tables, the latency is still >100
    // cycles on Ice Lake, and from other sources, seems to be high as well on
    // Adler Lake. Separately, it is (from
    // https://agner.org/optimize/instruction_tables.pdf) also high on AMD Zen 3
    // (~65). So just assume that it's this way for most x86_64 architectures.
    //
    // Also, loop several times here, following the guidelines in section 2.3.4
    // of the manual, "Pause latency in Skylake Client Microarchitecture".
    for (int yields = 0; yields < backoff; yields++) {
      PA_YIELD_PROCESSOR;
      tries++;
    }
    constexpr int kMaxBackoff = 16;
    backoff = XX_min(kMaxBackoff, backoff << 1);
  } while (tries < kSpinCount);

  LockSlow();
}

#if defined(OS_LINUX)
#define FUTEX_WAKE 1
#define FUTEX_PRIVATE_FLAG 128

void SpinningMutex::FutexWait() {
  // Save and restore errno.
  int saved_errno = errno;
  // Don't check the return value, as we will not be awaken by a timeout, since
  // none is specified.
  //
  // Ignoring the return value doesn't impact correctness, as this acts as an
  // immediate wakeup. For completeness, the possible errors for FUTEX_WAIT are:
  // - EACCES: state_ is not readable. Should not happen.
  // - EAGAIN: the value is not as expected, that is not |kLockedContended|, in
  //           which case retrying the loop is the right behavior.
  // - EINTR: signal, looping is the right behavior.
  // - EINVAL: invalid argument.
  //
  // Note: not checking the return value is the approach used in bionic and
  // glibc as well.
  //
  // Will return immediately if |state_| is no longer equal to
  // |kLockedContended|. Otherwise, sleeps and wakes up when |state_| may not be
  // |kLockedContended| anymore. Note that even without spurious wakeups, the
  // value of |state_| is not guaranteed when this returns, as another thread
  // may get the lock before we get to run.
  int err = syscall(SYS_futex, &state_, 0 /*Futex wait*/ | FUTEX_PRIVATE_FLAG,
                    kLockedContended, nullptr, nullptr, 0);

  if (err) {
    // These are programming error, check them.
    DCHECK(errno != EACCES);
    DCHECK(errno != EINVAL);
  }
  errno = saved_errno;
}

void SpinningMutex::FutexWake() {
  int saved_errno = errno;
  long retval = syscall(SYS_futex, &state_, FUTEX_WAKE | FUTEX_PRIVATE_FLAG,
                        1 /* wake up a single waiter */, nullptr, nullptr, 0);
  DCHECK(retval != -1);
  errno = saved_errno;
}

void SpinningMutex::LockSlow() {
  // If this thread gets awaken but another one got the lock first, then go back
  // to sleeping. See comments in |FutexWait()| to see why a loop is required.
  while (state_.exchange(kLockedContended, std::memory_order_acquire) != kUnlocked) {
    FutexWait();
  }
}
#endif

}  // namespace base
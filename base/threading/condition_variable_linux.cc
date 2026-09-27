// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/condition_variable.h>

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>

#include <limits.h>
#include <time.h>

namespace base {

void ConditionVariable::SleepWhileEquals(u32 seq) {
  int saved_errno = errno;
  syscall(SYS_futex, &seq_, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, seq, nullptr,
          nullptr, 0);
  errno = saved_errno;
}

void ConditionVariable::SleepWhileEqualsFor(u32 seq, TimeDelta timeout) {
  const i64 us = timeout.InMicroseconds();
  if (us <= 0)
    return;
  // FUTEX_WAIT takes a relative timeout.
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(us / 1000000);
  ts.tv_nsec = static_cast<long>((us % 1000000) * 1000);
  int saved_errno = errno;
  syscall(SYS_futex, &seq_, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, seq, &ts, nullptr,
          0);
  errno = saved_errno;
}

void ConditionVariable::WakeOne() {
  int saved_errno = errno;
  syscall(SYS_futex, &seq_, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, nullptr,
          nullptr, 0);
  errno = saved_errno;
}

void ConditionVariable::WakeAll() {
  int saved_errno = errno;
  syscall(SYS_futex, &seq_, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, nullptr,
          nullptr, 0);
  errno = saved_errno;
}

}  // namespace base

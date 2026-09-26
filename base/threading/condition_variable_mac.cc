// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/condition_variable.h>

#include <stdint.h>

namespace base {

// Same __ulock contract spinning_mutex.cc uses: UL_COMPARE_AND_WAIT blocks
// while the 32-bit word still equals the passed value.
extern "C" int __ulock_wait(uint32_t operation, void* addr, uint64_t value,
                            uint32_t timeout_us);
extern "C" int __ulock_wake(uint32_t operation, void* addr, uint64_t wake_value);

namespace {
constexpr uint32_t kUlCompareAndWait = 1;
constexpr uint32_t kUlfWakeAll = 0x00000100;
constexpr uint32_t kUlfNoErrno = 0x01000000;
}  // namespace

void ConditionVariable::WaitWhileEquals(u32 seq) {
  __ulock_wait(kUlCompareAndWait | kUlfNoErrno, &seq_, seq, 0);
}

void ConditionVariable::WaitWhileEqualsFor(u32 seq, TimeDelta timeout) {
  const i64 us = timeout.InMicroseconds();
  if (us <= 0)
    return;
  // A timeout of 0 means forever to __ulock_wait, so the longest finite wait
  // is UINT32_MAX microseconds; callers re-check and wait again past that.
  const uint32_t timeout_us =
      us > static_cast<i64>(UINT32_MAX) ? UINT32_MAX : static_cast<uint32_t>(us);
  __ulock_wait(kUlCompareAndWait | kUlfNoErrno, &seq_, seq, timeout_us);
}

void ConditionVariable::NotifyOne() {
  seq_.fetch_add(1, base::memory_order_relaxed);
  __ulock_wake(kUlCompareAndWait | kUlfNoErrno, &seq_, 0);
}

void ConditionVariable::NotifyAll() {
  seq_.fetch_add(1, base::memory_order_relaxed);
  __ulock_wake(kUlCompareAndWait | kUlfWakeAll | kUlfNoErrno, &seq_, 0);
}

}  // namespace base

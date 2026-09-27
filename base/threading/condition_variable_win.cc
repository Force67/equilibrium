// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <windows.h>
#include <base/threading/condition_variable.h>

// WaitOnAddress and friends live in Synchronization.lib (linked by the build).

namespace base {

void ConditionVariable::SleepWhileEquals(u32 seq) {
  ::WaitOnAddress(&seq_, &seq, sizeof(seq), INFINITE);
}

void ConditionVariable::SleepWhileEqualsFor(u32 seq, TimeDelta timeout) {
  const i64 us = timeout.InMicroseconds();
  if (us <= 0)
    return;
  // Round up to whole milliseconds so a short wait does not become a poll,
  // and stay below INFINITE; callers re-check and wait again past that.
  const i64 ms = (us + 999) / 1000;
  const DWORD wait_ms = ms >= static_cast<i64>(INFINITE) ? INFINITE - 1
                                                         : static_cast<DWORD>(ms);
  ::WaitOnAddress(&seq_, &seq, sizeof(seq), wait_ms);
}

void ConditionVariable::WakeOne() {
  ::WakeByAddressSingle(&seq_);
}

void ConditionVariable::WakeAll() {
  ::WakeByAddressAll(&seq_);
}

}  // namespace base

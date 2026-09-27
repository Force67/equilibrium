// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/atomic.h>
#include <base/export.h>
#include <base/memory/move.h>
#include <base/time/time.h>

namespace base {

// Sequence-counter condition variable over the platform's address wait
// (futex / __ulock / WaitOnAddress). Wait always loops on a predicate, so a
// spurious wakeup is harmless. Lock is anything with lock()/unlock(): a
// base::UniqueLock<base::Mutex> or the base::Mutex itself.
class BASE_EXPORT ConditionVariable {
 public:
  constexpr ConditionVariable() = default;
  BASE_NOCOPYMOVE(ConditionVariable);

  // |lock| must be held on entry and is held again on return.
  template <class Lock, class Pred>
  void Wait(Lock& lock, Pred pred) {
    while (!pred()) {
      // Read under the lock, before releasing it. A notifier that changes the
      // predicate after this point must bump the counter after we read it, so
      // WaitWhileEquals returns immediately instead of losing the wakeup.
      const u32 seq = seq_.load(base::memory_order_relaxed);
      lock.unlock();
      WaitWhileEquals(seq);
      lock.lock();
    }
  }

  // Blocks until a notify, until |timeout| has passed, or spuriously, like
  // std::condition_variable::wait_for without a predicate: callers re-check
  // their condition. |lock| must be held on entry and is held again on return.
  template <class Lock>
  void WaitFor(Lock& lock, TimeDelta timeout) {
    const u32 seq = seq_.load(base::memory_order_relaxed);
    lock.unlock();
    WaitWhileEqualsFor(seq, timeout);
    lock.lock();
  }

  // Waits until |pred| holds or |timeout| has passed; returns pred(), so
  // false means timed out. |lock| must be held on entry and is held again on
  // return.
  template <class Lock, class Pred>
  bool WaitFor(Lock& lock, TimeDelta timeout, Pred pred) {
    return WaitUntil(lock, TimeTicks::Now() + timeout, pred);
  }

  // As WaitFor, against a deadline on the monotonic clock.
  template <class Lock, class Pred>
  bool WaitUntil(Lock& lock, TimeTicks deadline, Pred pred) {
    while (!pred()) {
      const TimeTicks now = TimeTicks::Now();
      if (now >= deadline)
        return false;
      const u32 seq = seq_.load(base::memory_order_relaxed);
      lock.unlock();
      WaitWhileEqualsFor(seq, deadline - now);
      lock.lock();
    }
    return true;
  }

  // Free when nobody waits: the syscall is only made for a sleeper.
  void NotifyOne() {
    if (Advance())
      WakeOne();
  }
  void NotifyAll() {
    if (Advance())
      WakeAll();
  }

 private:
  // Moves the sequence on; true when a waiter may be asleep on it. Both this
  // and a waiter's registration are sequentially consistent, so either the
  // notifier sees the waiter or the waiter's sleep sees the new sequence.
  bool Advance() {
    seq_.fetch_add(1, base::memory_order_seq_cst);
    return waiters_.load(base::memory_order_seq_cst) != 0;
  }

  // Blocks while seq_ still equals |seq|. May return spuriously.
  void WaitWhileEquals(u32 seq) {
    waiters_.fetch_add(1, base::memory_order_seq_cst);
    SleepWhileEquals(seq);
    waiters_.fetch_sub(1, base::memory_order_relaxed);
  }
  // As WaitWhileEquals, giving up after |timeout|.
  void WaitWhileEqualsFor(u32 seq, TimeDelta timeout) {
    waiters_.fetch_add(1, base::memory_order_seq_cst);
    SleepWhileEqualsFor(seq, timeout);
    waiters_.fetch_sub(1, base::memory_order_relaxed);
  }

  // The platform's address wait and wake on seq_.
  void SleepWhileEquals(u32 seq);
  void SleepWhileEqualsFor(u32 seq, TimeDelta timeout);
  void WakeOne();
  void WakeAll();

  base::Atomic<u32> seq_{0};
  base::Atomic<u32> waiters_{0};
};

}  // namespace base

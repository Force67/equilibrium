// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/atomic.h>
#include <base/threading/condition_variable.h>
#include <base/threading/lock_guard.h>
#include <base/threading/mutex.h>
#include <base/threading/thread.h>
#include <base/time/time.h>

namespace {

using Lock = base::UniqueLock<base::Mutex>;

TEST(ConditionVariable, TruePredicateReturnsWithoutBlocking) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  Lock lock(mutex);
  int calls = 0;
  cv.Wait(lock, [&] {
    calls++;
    return true;
  });
  EXPECT_EQ(calls, 1);
  EXPECT_TRUE(lock.owns_lock());
}

TEST(ConditionVariable, WaitAcceptsTheMutexDirectly) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  bool ready = false;

  base::Thread producer(
      "cv-direct",
      [&] {
        base::LockGuard<base::Mutex> guard(mutex);
        ready = true;
        cv.NotifyOne();
      },
      /*start_now=*/true);

  mutex.lock();
  cv.Wait(mutex, [&] { return ready; });
  mutex.unlock();
  ASSERT_TRUE(producer.Join());
}

TEST(ConditionVariable, ProducerConsumerHandoff) {
  constexpr int kItems = 1000;
  base::Mutex mutex;
  base::ConditionVariable cv;
  // One-slot mailbox: 0 means empty, so the producer waits for the consumer
  // to drain each item and both sides block on every step.
  int slot = 0;
  long long sum = 0;

  base::Thread consumer(
      "cv-consumer",
      [&] {
        for (int i = 0; i < kItems; i++) {
          Lock lock(mutex);
          cv.Wait(lock, [&] { return slot != 0; });
          sum += slot;
          slot = 0;
          cv.NotifyAll();
        }
      },
      /*start_now=*/true);

  base::Thread producer(
      "cv-producer",
      [&] {
        for (int i = 1; i <= kItems; i++) {
          Lock lock(mutex);
          cv.Wait(lock, [&] { return slot == 0; });
          slot = i;
          cv.NotifyAll();
        }
      },
      /*start_now=*/true);

  ASSERT_TRUE(producer.Join());
  ASSERT_TRUE(consumer.Join());
  EXPECT_EQ(sum, static_cast<long long>(kItems) * (kItems + 1) / 2);
}

TEST(ConditionVariable, NotifyAllReleasesEveryWaiter) {
  constexpr int kWaiters = 8;
  base::Mutex mutex;
  base::ConditionVariable cv;
  bool go = false;
  int waiting = 0;
  base::Atomic<int> released{0};
  base::Thread* threads[kWaiters];

  for (int i = 0; i < kWaiters; i++) {
    threads[i] = new base::Thread(
        "cv-waiter",
        [&] {
          Lock lock(mutex);
          waiting++;
          cv.NotifyAll();
          cv.Wait(lock, [&] { return go; });
          released.fetch_add(1);
        },
        /*start_now=*/true);
  }

  {
    Lock lock(mutex);
    cv.Wait(lock, [&] { return waiting == kWaiters; });
    EXPECT_EQ(released.load(), 0);
    go = true;
  }
  cv.NotifyAll();

  for (int i = 0; i < kWaiters; i++) {
    EXPECT_TRUE(threads[i]->Join());
    delete threads[i];
  }
  EXPECT_EQ(released.load(), kWaiters);
}

TEST(ConditionVariable, TurnTakingStress) {
  // Threads take turns by ticket: each waits until the shared counter reaches
  // its own residue, bumps it, and wakes the rest. Every step hands off to a
  // blocked thread, so a lost wakeup shows up as a hang and a race as a
  // miscount.
  constexpr int kThreads = 4;
  constexpr int kRounds = 5000;
  base::Mutex mutex;
  base::ConditionVariable cv;
  int counter = 0;
  base::Thread* threads[kThreads];

  for (int t = 0; t < kThreads; t++) {
    threads[t] = new base::Thread(
        "cv-stress",
        [&, t] {
          for (int r = 0; r < kRounds; r++) {
            Lock lock(mutex);
            cv.Wait(lock, [&] { return counter % kThreads == t; });
            counter++;
            // Not NotifyOne: it may wake a thread whose turn it is not.
            cv.NotifyAll();
          }
        },
        /*start_now=*/true);
  }
  for (int t = 0; t < kThreads; t++) {
    EXPECT_TRUE(threads[t]->Join());
    delete threads[t];
  }
  EXPECT_EQ(counter, kThreads * kRounds);
}

TEST(ConditionVariable, NotifyOneCounterStress) {
  // Producers add work under the lock and wake one consumer each; consumers
  // drain it. Every item needs a wakeup, so a lost one leaves a consumer
  // asleep with work pending and the join hangs.
  constexpr int kProducers = 3;
  constexpr int kConsumers = 3;
  constexpr int kPerProducer = 20000;
  constexpr int kTotal = kProducers * kPerProducer;
  base::Mutex mutex;
  base::ConditionVariable cv;
  int pending = 0;
  int consumed = 0;
  base::Thread* threads[kProducers + kConsumers];

  for (int c = 0; c < kConsumers; c++) {
    threads[c] = new base::Thread(
        "cv-consume",
        [&] {
          for (;;) {
            Lock lock(mutex);
            cv.Wait(lock, [&] { return pending > 0 || consumed == kTotal; });
            if (pending == 0)
              return;
            pending--;
            if (++consumed == kTotal)
              cv.NotifyAll();
          }
        },
        /*start_now=*/true);
  }
  for (int p = 0; p < kProducers; p++) {
    threads[kConsumers + p] = new base::Thread(
        "cv-produce",
        [&] {
          for (int i = 0; i < kPerProducer; i++) {
            {
              Lock lock(mutex);
              pending++;
            }
            // Outside the lock on purpose: the sequence counter must still
            // catch a notify that lands between a waiter's check and its sleep.
            cv.NotifyOne();
          }
        },
        /*start_now=*/true);
  }
  for (auto* thread : threads) {
    EXPECT_TRUE(thread->Join());
    delete thread;
  }
  EXPECT_EQ(consumed, kTotal);
  EXPECT_EQ(pending, 0);
}

TEST(ConditionVariable, WaitForTimesOutWithoutNotify) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  Lock lock(mutex);
  const base::TimeTicks start = base::TimeTicks::Now();
  // Spurious wakes are allowed, so loop until the deadline like a caller would.
  const base::TimeTicks deadline = start + base::Milliseconds(20);
  while (base::TimeTicks::Now() < deadline)
    cv.WaitFor(lock, deadline - base::TimeTicks::Now());
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_GE((base::TimeTicks::Now() - start).InMilliseconds(), 20);
}

TEST(ConditionVariable, WaitForNonPositiveTimeoutReturns) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  Lock lock(mutex);
  cv.WaitFor(lock, base::TimeDelta());
  cv.WaitFor(lock, base::Milliseconds(-5));
  EXPECT_TRUE(lock.owns_lock());
}

TEST(ConditionVariable, WaitForWakesOnNotify) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  bool ready = false;
  Lock lock(mutex);
  base::Thread notifier(
      "cv-waitfor",
      [&] {
        base::LockGuard<base::Mutex> guard(mutex);
        ready = true;
        cv.NotifyAll();
      },
      /*start_now=*/true);
  // A minute-long timeout: returning well before it proves the notify woke us.
  const base::TimeTicks start = base::TimeTicks::Now();
  while (!ready)
    cv.WaitFor(lock, base::Seconds(60));
  EXPECT_LT((base::TimeTicks::Now() - start).InSeconds(), 30);
  lock.unlock();
  ASSERT_TRUE(notifier.Join());
}

TEST(ConditionVariable, WaitForWithPredicateTimesOut) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  Lock lock(mutex);
  const base::TimeTicks start = base::TimeTicks::Now();
  EXPECT_FALSE(cv.WaitFor(lock, base::Milliseconds(20), [] { return false; }));
  EXPECT_GE((base::TimeTicks::Now() - start).InMilliseconds(), 20);
  EXPECT_TRUE(lock.owns_lock());
}

TEST(ConditionVariable, WaitForWithPredicateReturnsOnceItHolds) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  bool ready = false;
  Lock lock(mutex);
  base::Thread notifier(
      "cv-pred",
      [&] {
        base::LockGuard<base::Mutex> guard(mutex);
        ready = true;
        cv.NotifyOne();
      },
      /*start_now=*/true);
  EXPECT_TRUE(cv.WaitFor(lock, base::Seconds(60), [&] { return ready; }));
  lock.unlock();
  ASSERT_TRUE(notifier.Join());
}

TEST(ConditionVariable, WaitUntilAPassedDeadlineChecksOnce) {
  base::Mutex mutex;
  base::ConditionVariable cv;
  Lock lock(mutex);
  int calls = 0;
  EXPECT_TRUE(cv.WaitUntil(lock, base::TimeTicks::Now() - base::Seconds(1),
                           [&] { return ++calls > 0; }));
  EXPECT_EQ(calls, 1);
  EXPECT_FALSE(cv.WaitUntil(lock, base::TimeTicks::Now() - base::Seconds(1),
                            [] { return false; }));
}

TEST(ConditionVariable, PingPongNeverLosesAWakeup) {
  // Each side waits for the other's turn; a notify that skipped its wake while
  // the other side was about to sleep would hang this test.
  base::Mutex mutex;
  base::ConditionVariable cv;
  int turn = 0;
  constexpr int kRounds = 100000;
  base::Thread other(
      "cv-pong",
      [&] {
        for (int i = 0; i < kRounds; i++) {
          Lock lock(mutex);
          cv.Wait(lock, [&] { return turn == 1; });
          turn = 0;
          lock.unlock();
          cv.NotifyAll();
        }
      },
      /*start_now=*/true);
  for (int i = 0; i < kRounds; i++) {
    {
      base::LockGuard<base::Mutex> guard(mutex);
      turn = 1;
    }
    cv.NotifyAll();
    Lock lock(mutex);
    cv.Wait(lock, [&] { return turn == 0; });
  }
  ASSERT_TRUE(other.Join());
}

}  // namespace

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/atomic.h>
#include <base/threading/thread.h>
#include <base/time/time.h>

namespace {

TEST(Thread, RunsTheFunctorOnAnotherThread) {
  base::Atomic<u32> ran{0};
  const u32 spawner = base::GetCurrentThreadIndex();
  base::Atomic<u32> observed{spawner};

  base::Thread thread(
      "runs-functor",
      [&] {
        observed.store(base::GetCurrentThreadIndex());
        ran.store(1);
      },
      /*start_now=*/true);

  ASSERT_TRUE(thread.good());
  ASSERT_TRUE(thread.Join());
  EXPECT_EQ(ran.load(), 1u);
  EXPECT_NE(observed.load(), spawner);
}

TEST(Thread, JoinWaitsForCompletion) {
  // The functor publishes |done| last, so a Join that returned early would
  // race and the read below would see 0.
  base::Atomic<int> counter{0};
  base::Atomic<int> done{0};

  base::Thread thread(
      "join-waits",
      [&] {
        for (int i = 0; i < 10000; i++)
          counter.fetch_add(1);
        done.store(1);
      },
      /*start_now=*/true);

  ASSERT_TRUE(thread.Join());
  EXPECT_EQ(done.load(), 1);
  EXPECT_EQ(counter.load(), 10000);
}

TEST(Thread, JoinOnAnUnstartedThreadFails) {
  base::Thread thread("never-started", [] {});
  EXPECT_FALSE(thread.good());
  EXPECT_FALSE(thread.Join());
}

TEST(Thread, JoinIsNotRepeatable) {
  base::Thread thread("join-once", [] {}, /*start_now=*/true);
  ASSERT_TRUE(thread.Join());
  // Join clears the handle, so a second one reports there is nothing to wait
  // for rather than waiting on a handle the OS may have recycled.
  EXPECT_FALSE(thread.good());
  EXPECT_FALSE(thread.Join());
}

TEST(Thread, SeveralThreadsJoinIndependently) {
  constexpr int kThreads = 8;
  base::Atomic<int> total{0};
  base::Thread* threads[kThreads];

  for (int i = 0; i < kThreads; i++) {
    threads[i] = new base::Thread(
        "fanout", [&total] { total.fetch_add(1); }, /*start_now=*/true);
  }
  for (int i = 0; i < kThreads; i++) {
    EXPECT_TRUE(threads[i]->Join());
    delete threads[i];
  }
  EXPECT_EQ(total.load(), kThreads);
}

TEST(Thread, SleepForMicrosecondsSleepsAtLeastAsLongAsAsked) {
  // Schedulers overshoot freely and the clock is coarse, so the assertion is
  // one-sided: the sleep must not return early.
  constexpr u64 kRequestedUs = 20000;  // 20 ms
  const i64 before = base::TickClock::NowNs();
  base::SleepForMicroseconds(kRequestedUs);
  const i64 elapsed_ns = base::TickClock::NowNs() - before;

  EXPECT_GE(elapsed_ns, static_cast<i64>(kRequestedUs) * 1000);
}

TEST(Thread, SleepForMillisecondsMatchesTheMicrosecondForm) {
  const i64 before = base::TickClock::NowNs();
  base::SleepForMilliseconds(10);
  const i64 elapsed_ns = base::TickClock::NowNs() - before;

  EXPECT_GE(elapsed_ns, 10 * 1000 * 1000);
}

TEST(Thread, SleepForZeroReturns) {
  base::SleepForMicroseconds(0);
  SUCCEED();
}

TEST(Thread, YieldCurrentThreadReturns) {
  // Yielding has no observable result to assert on; what matters is that it
  // is callable and comes back.
  for (int i = 0; i < 100; i++)
    base::YieldCurrentThread();
  SUCCEED();
}

TEST(Thread, SleepUnblocksAWaitingThread) {
  // Exercises sleep on a spawned thread rather than the gtest one, which is
  // where thread_pool actually calls it.
  base::Atomic<int> woke{0};
  base::Thread thread(
      "sleeper",
      [&] {
        base::SleepForMicroseconds(1000);
        woke.store(1);
      },
      /*start_now=*/true);

  ASSERT_TRUE(thread.Join());
  EXPECT_EQ(woke.load(), 1);
}

}  // namespace

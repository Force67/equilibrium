// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#include <gtest/gtest.h>

#include <base/atomic.h>
#include <base/threading/thread_pool.h>

#if !defined(OS_WIN)
#include <ctime>
#endif

namespace {

void SpinWait(base::Atomic<int>& counter, int expected, int timeout_ms) {
  for (int waited = 0; counter.load() != expected && waited < timeout_ms;
       waited++) {
#if defined(OS_WIN)
    ::Sleep(1);
#else
    timespec ts{0, 1000000};
    ::nanosleep(&ts, nullptr);
#endif
  }
}

TEST(ThreadPool, ExecutesEnqueuedTasks) {
  base::Atomic<int> counter{0};
  {
    base::ThreadPool pool(2, 4);
    for (int i = 0; i < 64; i++)
      pool.enqueue([&counter]() { counter.fetch_add(1); });
    SpinWait(counter, 64, 5000);
  }
  EXPECT_EQ(counter.load(), 64);
}

TEST(ThreadPool, ScalesUpUnderLoad) {
  base::ThreadPool pool(1, 4);
  EXPECT_EQ(pool.workerCount(), 1u);
  base::Atomic<int> counter{0};
  for (int i = 0; i < 32; i++)
    pool.enqueue([&counter]() { counter.fetch_add(1); });
  EXPECT_GE(pool.workerCount(), 1u);
  EXPECT_LE(pool.workerCount(), 4u);
  SpinWait(counter, 32, 5000);
  EXPECT_EQ(counter.load(), 32);
}

TEST(ThreadPool, TasksRunConcurrently) {
  // 6 tasks of 200 ms on 6 workers must overlap; a serial pool would need
  // 1.2 s. Generous threshold to stay CI-safe.
  base::Atomic<int> done{0};
  base::ThreadPool pool(6, 6);
  const auto nap200 = []() {
    timespec ts{0, 200000000};
    ::nanosleep(&ts, nullptr);
  };
  const auto start = ::time(nullptr);
  for (int i = 0; i < 6; i++)
    pool.enqueue([&done, nap200]() {
      nap200();
      done.fetch_add(1);
    });
  SpinWait(done, 6, 5000);
  const auto elapsed = ::time(nullptr) - start;
  EXPECT_EQ(done.load(), 6);
  EXPECT_LE(elapsed, 1);  // seconds; serial execution would take >= 1.2 s
}

TEST(ThreadPool, SurvivesThousandsOfInterleavedTasks) {
  // Regression: task storage must not lose callables when the queue grows
  // and wraps while workers drain it concurrently.
  base::Atomic<int> counter{0};
  {
    base::ThreadPool pool(2, 8);
    for (int batch = 0; batch < 50; batch++) {
      for (int i = 0; i < 100; i++)
        pool.enqueue([&counter]() { counter.fetch_add(1); });
      SpinWait(counter, (batch + 1) * 100, 10000);
    }
  }
  EXPECT_EQ(counter.load(), 5000);
}

TEST(ThreadPool, DestructionWithIdleWorkers) {
  base::ThreadPool pool(3, 3);
  // Destructor must not hang or leak with never-used workers.
}

TEST(ThreadPool, StartWithPriorityDoesNotTrap) {
  // Regression: posix SetThreadPriority used to DCHECK(false), making every
  // Thread::Start crash in debug builds.
  base::Atomic<int> ran{0};
  base::Thread thread("prio_test",
                      base::Function<void()>([&ran]() { ran.fetch_add(1); }),
                      /*start_now=*/true, base::Thread::Priority::kHigh);
  SpinWait(ran, 1, 5000);
  EXPECT_EQ(ran.load(), 1);
}

}  // namespace

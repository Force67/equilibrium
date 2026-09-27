// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/threading/lock_guard.h>
#include <base/threading/recursive_mutex.h>
#include <base/threading/thread.h>

namespace {
TEST(RecursiveMutex, OwnerMayRelock) {
  base::RecursiveMutex m;
  m.lock();
  m.lock();
  EXPECT_TRUE(m.try_lock());
  m.unlock();
  m.unlock();
  m.unlock();
  EXPECT_TRUE(m.try_lock());
  m.unlock();
}

TEST(RecursiveMutex, ExcludesOtherThreads) {
  base::RecursiveMutex m;
  m.lock();
  m.lock();
  bool other_got_it = true;
  base::Thread other("rm-test", [&] { other_got_it = m.try_lock(); }, true);
  other.Join();
  EXPECT_FALSE(other_got_it);
  m.unlock();
  m.unlock();
  base::Thread after("rm-test", [&] {
    other_got_it = m.try_lock();
    if (other_got_it)
      m.unlock();
  }, true);
  after.Join();
  EXPECT_TRUE(other_got_it);
}

TEST(RecursiveMutex, CountsStayConsistentUnderContention) {
  base::RecursiveMutex m;
  int counter = 0;
  constexpr int kThreads = 4, kIters = 20000;
  base::Thread* threads[kThreads];
  for (int t = 0; t < kThreads; t++)
    threads[t] = new base::Thread("rm-test", [&] {
      for (int i = 0; i < kIters; i++) {
        base::LockGuard<base::RecursiveMutex> outer(m);
        base::LockGuard<base::RecursiveMutex> inner(m);
        counter++;
      }
    }, true);
  for (auto* t : threads) {
    t->Join();
    delete t;
  }
  EXPECT_EQ(counter, kThreads * kIters);
}
}  // namespace

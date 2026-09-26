// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// The lock headers come first, before anything else can pull in <windows.h>:
// they must not bring its macros along, or every file holding a mutex has names
// like MemoryBarrier or near rewritten under it.
#include <base/threading/mutex.h>
#include <base/threading/spinning_mutex.h>

#if defined(MemoryBarrier) || defined(near) || defined(far) || defined(min) || defined(max)
#error "a base lock header leaked a Windows macro"
#endif

#include <gtest/gtest.h>

#include <base/atomic.h>
#include <base/threading/lock_guard.h>
#include <base/threading/thread.h>

namespace {

TEST(SpinningMutex, TryFailsWhileHeld) {
  base::SpinningMutex mutex;
  mutex.lock();
  EXPECT_FALSE(mutex.try_lock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

TEST(SpinningMutex, SerializesIncrements) {
  constexpr int kThreads = 4;
  constexpr int kIterations = 20000;
  base::SpinningMutex mutex;
  int counter = 0;
  base::Thread* threads[kThreads];
  for (auto& t : threads) {
    t = new base::Thread(
        "mutex-inc",
        [&] {
          for (int i = 0; i < kIterations; i++) {
            base::LockGuard<base::SpinningMutex> guard(mutex);
            ++counter;
          }
        },
        /*start_now=*/true);
  }
  for (auto* t : threads) {
    EXPECT_TRUE(t->Join());
    delete t;
  }
  EXPECT_EQ(counter, kThreads * kIterations);
}

TEST(SharedMutex, ReadersShareWritersExclude) {
  base::SharedMutex mutex;
  mutex.lock_shared();
  EXPECT_TRUE(mutex.try_lock_shared());
  EXPECT_FALSE(mutex.try_lock());
  mutex.unlock_shared();
  mutex.unlock_shared();

  mutex.lock();
  EXPECT_FALSE(mutex.try_lock_shared());
  EXPECT_FALSE(mutex.try_lock());
  mutex.unlock();
  EXPECT_TRUE(mutex.try_lock());
  mutex.unlock();
}

}  // namespace

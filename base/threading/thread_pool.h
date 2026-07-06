// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/atomic.h>
#include <base/export.h>
#include <base/threading/thread.h>
#include <base/threading/spinning_mutex.h>
#include <base/containers/deque.h>
#include <base/containers/vector.h>

namespace base {

// Fixed-elasticity worker pool. Spawns minThreads immediately and grows up
// to maxThreads when the queue backs up. Workers poll the queue and nap
// briefly when idle (BASE has no condition variables). The destructor
// drains nothing: pending tasks are dropped, running tasks finish.
class BASE_EXPORT ThreadPool {
 public:
  ThreadPool(size_t minThreads, size_t maxThreads);
  ~ThreadPool();

  void enqueue(base::Function<void()> task);
  void adjustThreadCount();

  size_t workerCount() const { return workers.size(); }
  size_t pendingTasks();

 private:
  void workerThreadFunction();
  void scaleUp(size_t target);

  base::SpinningMutex queueMutex;
  base::SimpleDeque<base::Function<void()>> taskQueue;

  base::Vector<base::Thread*> workers;
  size_t minThreads, maxThreads;
  base::Atomic<bool> stop;
  base::Atomic<size_t> activeWorkers;
};
}  // namespace base

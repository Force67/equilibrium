// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/atomic.h>
#include <base/export.h>
#include <base/threading/thread.h>
#include <base/threading/spinning_mutex.h>
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

  size_t workerCount() const;
  size_t pendingTasks();

 private:
  void workerThreadFunction();
  // Adds workers until there are |target| of them. Call with workersMutex
  // held: it both reads and grows |workers|.
  void scaleUp(size_t target);

  // LIFO stack rather than a deque: tasks carry no ordering contract, and
  // Vector moves the type-erased callables instead of copy-assigning them.
  base::SpinningMutex queueMutex;
  base::Vector<base::Function<void()>> taskQueue;

  // The worker list has its own lock rather than sharing queueMutex, so that
  // spawning a thread never stalls the workers polling the queue. Every read
  // and every write of |workers| happens under it, including the size checks
  // that decide whether to grow: two concurrent enqueues that both read an
  // unsynchronized size pick the same target and push into the same Vector.
  mutable base::SpinningMutex workersMutex;
  base::Vector<base::Thread*> workers;
  size_t minThreads, maxThreads;
  base::Atomic<bool> stop;
  base::Atomic<size_t> activeWorkers;
};
}  // namespace base

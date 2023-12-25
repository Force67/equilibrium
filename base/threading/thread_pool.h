// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <queue>
#include <base/atomic.h>
#include <base/threading/thread.h>
#include <base/containers/vector.h>

namespace base {

class ThreadPool {
 public:
  ThreadPool(size_t minThreads, size_t maxThreads);
  ~ThreadPool();

  void enqueue(base::Function<void()> task);
  void adjustThreadCount();

 private:
  void workerThreadFunction();
  void scaleUp(size_t target);

  // Custom task queue and synchronization primitives
  std::queue<base::Function<void()>> taskQueue;

  base::Vector<base::Thread*> workers;
  size_t minThreads, maxThreads;
  base::Atomic<bool> stop;
};
}  // namespace base
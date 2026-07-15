// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "thread_pool.h"

#include <base/threading/lock_guard.h>

#include <chrono>
#include <thread>

namespace base {
namespace {
// Idle nap between queue polls; long enough to keep idle workers near 0%
// CPU, short enough to not matter for task latency.
void NapBriefly() {
  std::this_thread::sleep_for(std::chrono::microseconds(500));
}
}  // namespace

ThreadPool::ThreadPool(size_t min_threads, size_t max_threads)
    : minThreads(min_threads), maxThreads(max_threads), stop(false), activeWorkers(0) {
  BASE_DCHECK(minThreads <= maxThreads, "ThreadPool: min > max");
  scaleUp(minThreads);
}

ThreadPool::~ThreadPool() {
  stop.store(true);
  while (activeWorkers.load() > 0)
    NapBriefly();
  for (base::Thread* worker : workers)
    delete worker;
}

void ThreadPool::enqueue(base::Function<void()> task) {
  size_t depth;
  {
    base::NonOwningScopedLockGuard<base::SpinningMutex> lock(queueMutex);
    taskQueue.push_back(base::move(task));
    depth = taskQueue.size();
  }
  // Grow while the queue outpaces the workers.
  if (depth > workers.size() && workers.size() < maxThreads)
    scaleUp(workers.size() + 1);
}

void ThreadPool::adjustThreadCount() {
  // Growth happens in enqueue; nothing shrinks (workers are cheap when
  // idle and Thread has no join to retire them safely mid-run).
}

size_t ThreadPool::pendingTasks() {
  base::NonOwningScopedLockGuard<base::SpinningMutex> lock(queueMutex);
  return taskQueue.size();
}

void ThreadPool::workerThreadFunction() {
  while (!stop.load()) {
    base::Function<void()> task;
    bool has_task = false;
    {
      base::NonOwningScopedLockGuard<base::SpinningMutex> lock(queueMutex);
      if (!taskQueue.empty()) {
        task = base::move(taskQueue.back());
        taskQueue.pop_back();
        has_task = true;
      }
    }
    if (has_task)
      task();
    else
      NapBriefly();
  }
  activeWorkers.fetch_sub(1);
}

void ThreadPool::scaleUp(size_t target) {
  for (size_t i = workers.size(); i < target; ++i) {
    activeWorkers.fetch_add(1);
    base::Thread* worker = new base::Thread(
        "WorkerThread", [this]() { this->workerThreadFunction(); }, true);
    workers.push_back(worker);
  }
}

}  // namespace base

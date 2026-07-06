// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "thread_pool.h"

#include <base/threading/lock_guard.h>

#if defined(OS_WIN)
#include <base/win/minwin.h>
#else
#include <ctime>
#endif

namespace base {
namespace {
// Idle nap between queue polls; long enough to keep idle workers near 0%
// CPU, short enough to not matter for task latency.
void NapBriefly() {
#if defined(OS_WIN)
  ::Sleep(1);
#else
  timespec ts{0, 500000};  // 0.5 ms
  ::nanosleep(&ts, nullptr);
#endif
}
}  // namespace

ThreadPool::ThreadPool(size_t min_threads, size_t max_threads)
    : minThreads(min_threads),
      maxThreads(max_threads),
      stop(false),
      activeWorkers(0) {
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
        task = base::move(taskQueue.front());
        taskQueue.pop_front();
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

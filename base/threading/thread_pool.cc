// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "thread_pool.h"

namespace base {
void ThreadPool::enqueue(base::Function<void()> task) {
  // Lock the task queue
  // Example: MyLock lock(queueMutex);

  //taskQueue.push(task);
  // Notify a waiting thread (using your condition variable equivalent)
  // Example: condition.notifyOne();
}

void ThreadPool::workerThreadFunction() {
#if 0
  while (!stop) {
    base::Function<void()> task;
    {
      // Lock the task queue
      // Example: MyLock lock(queueMutex);
      while (taskQueue.empty() && !stop) {
        // Wait for a task (using your condition variable equivalent)
        // Example: condition.wait(lock);
      }
      if (stop)
        break;
      task = taskQueue.front();
      taskQueue.pop();
    }
    task();  // Execute the task
  }
#endif
}

void ThreadPool::scaleUp(size_t target) {
  for (size_t i = workers.size(); i < target; ++i) {
    base::Thread* worker = new base::Thread(
        "WorkerThread", [this]() { this->workerThreadFunction(); }, true);
    workers.push_back(worker);
  }
}

}  // namespace base
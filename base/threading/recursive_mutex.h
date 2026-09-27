// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/atomic.h>
#include <base/threading/mutex.h>

namespace base {

// A base::Mutex its owning thread may lock again; every lock needs its unlock.
class RecursiveMutex {
 public:
  void lock() {
    const mem_size self = ThreadTag();
    if (owner_.load(base::memory_order_relaxed) == self) {
      depth_++;
      return;
    }
    mutex_.lock();
    owner_.store(self, base::memory_order_relaxed);
    depth_ = 1;
  }

  bool try_lock() {
    const mem_size self = ThreadTag();
    if (owner_.load(base::memory_order_relaxed) == self) {
      depth_++;
      return true;
    }
    if (!mutex_.try_lock())
      return false;
    owner_.store(self, base::memory_order_relaxed);
    depth_ = 1;
    return true;
  }

  void unlock() {
    if (--depth_)
      return;
    owner_.store(0, base::memory_order_relaxed);
    mutex_.unlock();
  }

 private:
  // Unique per live thread, without asking the OS.
  static mem_size ThreadTag() {
    static thread_local char tag;
    return reinterpret_cast<mem_size>(&tag);
  }

  Mutex mutex_;
  Atomic<mem_size> owner_{0};
  mem_size depth_ = 0;
};

}  // namespace base

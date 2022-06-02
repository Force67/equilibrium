// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/thread.h>

namespace base {

Thread::Thread(const base::StringRef ref, const Thread::Priority prio)
    : thread_name_(ref.c_str()) {
  parent_thread_index_ = base::GetCurrentThreadIndex();

  handle_ = Thread::Spawn();
  if (handle_)
    base::SetThreadPriority(handle_, prio);
}

// right now, the user has to implement it.
u32 Thread::Run() {
  DEBUG_TRAP;
  return 0;
}

void Thread::SetName(const base::StringRef name) {
  thread_name_ = name.data();
  base::SetThreadName(handle_, name.data());
}

void Thread::ApplyName() {
  base::SetThreadName(handle_, thread_name_.c_str());
}

void Thread::SetPrio(const Thread::Priority prio) {
  base::SetThreadPriority(handle_, prio);
}

Thread::Priority Thread::GetPrio() const {
  return base::GetThreadPriority(handle_);
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/thread.h>

namespace base {

Thread::Thread(const base::StringRef ref,
               base::Function<void()> functor,
               bool start_now,
               const Thread::Priority prio)
    : thread_name_(ref.c_str(), ref.length()), run_functor_(base::move(functor)) {
  parent_thread_index_ = base::GetCurrentThreadIndex();

  if (start_now) {
    Start(prio);
  }
}

bool Thread::Start(const Priority prio) {
  handle_data_ = Thread::Spawn();
  if (good()) {
    base::SetThreadPriority(handle_data_, prio);
    return true;
  }
  return false;
}

u32 Thread::Run() {
  run_functor_();
  return 0;
}

void Thread::SetName(const base::StringRef name) {
  thread_name_ = base::String(name.data(), name.length());
  base::SetThreadName(handle_data_, thread_name_.c_str());
}

void Thread::ApplyName() {
  // Runs on the freshly spawned thread, racing the parent's assignment of
  // handle_data_ (garbage until Spawn() returns there). Name the calling
  // thread through its own handle instead.
  base::SetThreadName(base::GetCurrentThreadHandle(), thread_name_.c_str());
}

void Thread::SetPrio(const Thread::Priority prio) {
  base::SetThreadPriority(handle_data_, prio);
}

Thread::Priority Thread::GetPrio() const {
  return base::GetThreadPriority(handle_data_);
}
}  // namespace base
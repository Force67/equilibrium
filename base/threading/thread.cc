// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/thread.h>

namespace base {

Thread::Thread(const base::StringRef ref, const Thread::Priority prio) {
  handle_ = base::SpawnThread(ref, static_cast<void*>(this));
  if (handle_)
    base::SetThreadPriority(handle_, prio);
}

// thin and light abstractions
void Thread::SetName(const base::StringRef name) {
  base::SetThreadName(handle_, name.data());
}

void Thread::SetPrio(const Thread::Priority prio) {
  base::SetThreadPriority(handle_, prio);
}

Thread::Priority Thread::GetPrio() const {
  return base::GetThreadPriority(handle_);
}
}  // namespace base
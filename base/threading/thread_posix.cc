// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <pthread.h>
#include <sys/syscall.h>

#include <base/check.h>
#include <base/threading/thread.h>
#include <base/allocator/memory_coordinator.h>

namespace base {

namespace {
void* ThreadFunc(void* user_param) {
    
  auto* thread = static_cast<base::Thread*>(user_param);
  thread->ApplyName();
  thread->Run();

  return nullptr;
}
}  // namespace

Thread::Handle Thread::Spawn() {
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);

  pthread_t handle;
  auto ec = ::pthread_create(&handle, &attributes, ThreadFunc, this);
  if (ec == 0) {
    return reinterpret_cast<Thread::Handle>(handle);
  }

  BUGCHECK(last_error, "pthread_create() error");
  return nullptr;
}

void SetThreadPriority(Thread::Handle handle, Thread::Priority new_priority) {
  DCHECK(false);
}

const i32 GetNativeThreadPriority(Thread::Handle handle) {
  sched_param param;
  i32 priority;
  i32 policy;

  /* scheduling parameters of target thread */
  if (::pthread_getschedparam(reinterpret_cast<pthread_t>(handle), &policy,
                              &param) != 0)
    return UINT_MAX;

  return param.sched_priority;
}

const Thread::Priority GetThreadPriority(Thread::Handle handle) {
  DCHECK(false);

  return Thread::Priority::kLow;
}

// Sets the debugger-visible name of the current thread.
bool SetThreadName(Thread::Handle handle, const char* name) {
  return ::pthread_setname_np(reinterpret_cast<pthread_t>(handle), name) == 0;
}

u32 GetCurrentThreadIndex() {
  return ::syscall(__NR_gettid);
}

Thread::Handle GetCurrentThreadHandle() {
  return reinterpret_cast<Thread::Handle>(::pthread_self());
}
}  // namespace base
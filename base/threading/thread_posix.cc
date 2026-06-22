// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <climits>

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

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

// glibc's pthread_t is an integer, but Apple's is an opaque pointer, so the two
// directions need different casts to round-trip through Thread::Handle's
// integral pthread_ slot.
inline pointer_size PthreadToHandle(pthread_t thread) {
#if defined(__APPLE__)
  return reinterpret_cast<pointer_size>(thread);
#else
  return static_cast<pointer_size>(thread);
#endif
}

inline pthread_t HandleToPthread(pointer_size handle) {
#if defined(__APPLE__)
  return reinterpret_cast<pthread_t>(handle);
#else
  return static_cast<pthread_t>(handle);
#endif
}
}  // namespace

Thread::Handle Thread::Spawn() {
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);

  pthread_t handle{};
  auto ec = ::pthread_create(&handle, &attributes, ThreadFunc, this);
  if (ec == 0) {
    return {.pthread_ = PthreadToHandle(handle)};
  }

  // handle may be garbrage when the thread creation fails, so we ensure that
  // null is returned
  BASE_BUGCHECK(ec, "pthread_create() error");
  return {.pthread_ = 0};
}

void SetThreadPriority(Thread::Handle handle, Thread::Priority new_priority) {
  BASE_DCHECK(false);
  sched_param param{.sched_priority = static_cast<int>(new_priority)};
  pthread_setschedparam(HandleToPthread(handle.pthread_), SCHED_OTHER, &param);
}

const i32 GetNativeThreadPriority(Thread::Handle handle) {
  sched_param param;
  i32 priority;
  i32 policy;

  /* scheduling parameters of target thread */
  if (::pthread_getschedparam(HandleToPthread(handle.pthread_), &policy, &param) != 0)
    return UINT_MAX;  // invalid cast to i32

  return param.sched_priority;
}

const Thread::Priority GetThreadPriority(Thread::Handle handle) {
  BASE_DCHECK(false);

  return Thread::Priority::kLow;
}

// Sets the debugger-visible name of the current thread.
bool SetThreadName(Thread::Handle handle, const char* name) {
#if defined(__APPLE__)
  // macOS can only name the calling thread; ApplyName runs on the thread
  // itself, so the handle is unused here.
  (void)handle;
  return ::pthread_setname_np(name) == 0;
#else
  return ::pthread_setname_np(handle.pthread_, name) == 0;
#endif
}

u32 GetCurrentThreadIndex() {
#if defined(__APPLE__)
  uint64_t tid = 0;
  ::pthread_threadid_np(nullptr, &tid);
  return static_cast<u32>(tid);
#else
  return ::syscall(__NR_gettid);
#endif
}

Thread::Handle GetCurrentThreadHandle() {
  return {.pthread_ = PthreadToHandle(::pthread_self())};
}
}  // namespace base

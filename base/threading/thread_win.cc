// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/feature.h>
#include <base/check.h>
#include <base/threading/thread.h>
#include <base/text/code_convert.h>
#include <base/win/minwin.h>
#include <base/dynamic_library.h>
#include <base/allocator/memory_coordinator.h>

namespace base {

namespace {
DWORD WINAPI ThreadFunc(void* user_param) {
  auto* thread = static_cast<base::Thread*>(user_param);
  // its best practice to set the thread name on the thread itself on windows
  thread->ApplyName();

  return thread->Run();
}
}  // namespace

Thread::Handle Thread::Spawn() {
  void* thread_handle =
      ::CreateThread(nullptr, 0, ThreadFunc, static_cast<LPVOID>(this), 0, 0);
  if (!thread_handle) {
    DWORD last_error = ::GetLastError();
    switch (last_error) {
      case ERROR_NOT_ENOUGH_MEMORY:
      case ERROR_OUTOFMEMORY:
      case ERROR_COMMITMENT_LIMIT:
        base::InvokeOutOfMemoryHandler();
        break;
      default:
        BUGCHECK(last_error, "CreateThread() error");
        break;
    }
  }
  return thread_handle;
}

void SetThreadPriority(Thread::Handle handle, Thread::Priority new_priority) {
  int windows_priority = 0;
  switch (new_priority) {
    case Thread::Priority::kLow:
      windows_priority = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case Thread::Priority::kNormal:
      windows_priority = THREAD_PRIORITY_NORMAL;
      break;
    case Thread::Priority::kHigh:
      windows_priority = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case Thread::Priority::kVeryHigh:
      windows_priority = THREAD_PRIORITY_HIGHEST;
      break;
    default:
      windows_priority = THREAD_PRIORITY_NORMAL;
      break;
  }
  ::SetThreadPriority(handle, windows_priority);
}

const Thread::Priority GetThreadPriority(Thread::Handle handle) {
  switch (::GetThreadPriority(handle)) {
    default:
    case THREAD_PRIORITY_BELOW_NORMAL:
      return Thread::Priority::kLow;
    case THREAD_PRIORITY_NORMAL:
      return Thread::Priority::kNormal;
    case THREAD_PRIORITY_ABOVE_NORMAL:
      return Thread::Priority::kHigh;
    case THREAD_PRIORITY_HIGHEST:
      return Thread::Priority::kVeryHigh;
  }
}

const i32 GetNativeThreadPriority(Thread::Handle handle) {
  return ::GetThreadPriority(handle);
}

// Sets the debugger-visible name of the current thread.
bool SetThreadName(Thread::Handle handle, const char* name) {
  // TODO: optimize for existing library case
  const base::DynamicLibrary kernel_base("kernelbase.dll");
  auto set_thread_desc =
      kernel_base.FindSymbol<HRESULT(WINAPI*)(void*, const wchar_t*)>(
          "SetThreadDescription");

  if (!set_thread_desc)
    return false;

  auto wide = base::ASCIIToWide(name);
  return SUCCEEDED(set_thread_desc(handle, wide.c_str()));
}

u32 GetCurrentThreadIndex() {
  return ::GetCurrentThreadId();
}

Thread::Handle GetCurrentThreadHandle() {
  return ::GetCurrentThread();
}
}  // namespace base
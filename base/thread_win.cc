// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <VersionHelpers.h>
#include <base/thread.h>
#include <base/feature.h>
#include <base/string/code_convert.h>
#include <base/dynamic_library.h>

namespace base {

namespace {
const base::Feature kUseLegacyThreadNaming{
    "base:threading:use_legacy_thread_naming"};

void LegacySetThreadName(const char* thread_name, DWORD thread_id = -1) {
  static constexpr DWORD kMSVCException = 0x406D1388;

#pragma pack(push, 8)
  struct THREADNAME_INFO {
    DWORD dwType;      // must be 0x1000
    LPCSTR szName;     // pointer to name (in user addr space)
    DWORD dwThreadID;  // thread ID (-1=caller thread)
    DWORD dwFlags;     // reserved for future use, must be zero
  } info;
#pragma pack(pop)

  info.dwType = 0x1000;
  info.szName = thread_name;
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    RaiseException(kMSVCException, 0, sizeof(info) / sizeof(ULONG_PTR),
                   (ULONG_PTR*)&info);
  } __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
}
}  // namespace

void SetCurrentThreadPriority(Thread::Priority new_priority) {
  auto handle = GetCurrentThread();
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
  SetThreadPriority(handle, windows_priority);
}

// Sets the debugger-visible name of the current thread.
void SetCurrentThreadName(const char* name) {
  auto set_thread_description = reinterpret_cast<HRESULT(WINAPI*)(HANDLE, PCWSTR)>(GetProcAddress(
      GetModuleHandle(L"kernelbase.dll"), "SetThreadDescription"));

  if (kUseLegacyThreadNaming || !set_thread_description) {
    return LegacySetThreadName(name);
  }

  auto wide = base::ASCIIToWide(name);
  set_thread_description(::GetCurrentThread(), wide.c_str());
}

u32 GetCurrentThreadIndex() {
  return ::GetCurrentThreadId();
}
}  // namespace base
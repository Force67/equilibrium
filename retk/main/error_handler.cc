// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <main/error_handler.h>

#include <base/check.h>
#include <base/allocator/memory_errror.h>

#if defined(OS_WIN)
#include <base/win/minwin.h>

extern "C" __declspec(dllimport) int __stdcall MessageBoxW(HWND hWnd,
                                                const wchar_t* lpText,
                                                const wchar_t* lpCaption,
                                                UINT uType);
#endif

namespace main {

namespace {
void OutOfMemoryHandler(void* user_pointer, const char* reason) {
  DEBUG_TRAP;
}

void CheckHandler(const char* message,
                  const char* file_name,
                  const char* function,
                  const char* msg) {
#if defined(OS_WIN)
  ::MessageBoxW(nullptr, L"An assertion failed. quitting.", L"RETK", 0x00000010L);
#endif
}
}  // namespace

void InstallErrorHandlers() {
  base::SetOutOfMemoryHandler(OutOfMemoryHandler, nullptr);
  base::SetCheckHandler(CheckHandler);
}
}  // namespace main
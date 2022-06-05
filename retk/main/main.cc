// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "application.h"

// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

#include <base/check.h>
#include <base/allocator/memory_coordinator.h>
#include <base/threading/thread.h>
#include <base/text/code_convert.h>

// TODO(Vince): Remove
#include <Windows.h>

namespace {
void HandleOOM(void* user_pointer, base::MemoryCoordinator&) {
  BUGCHECK("OOM encountered");
}

void TKLogHandler(void* user_pointer, base::LogLevel level, const char* msg) {
#if defined(OS_WIN) || defined(CONFIG_DEBUG)
  {
    // const char* src, size_t src_len, base::StringW* output
    base::StringW wide_text;
    BUGCHECK(base::UTF8ToWide(msg, std::strlen(msg), &wide_text));
    OutputDebugStringW(wide_text.c_str());
  }
#endif
}

void AssertHandler(const char*, const char*, const char*) {
#if defined(OS_WIN)
  MessageBoxW(nullptr, L"An assertion failed. quitting.", L"RETK", MB_ICONSTOP);
#endif
}
}  // namespace

int main() {
  // we install these as early as possible so we can catch failures during
  // app construction
  {
    base::SetOutOfMemoryHandler(HandleOOM, nullptr);
    base::SetLogHandler(TKLogHandler, nullptr);
    base::SetAssertHandler(AssertHandler);
    base::SetCurrentThreadName("Main");
  }

  // keep the stack free
  auto app{std::make_unique<Application>()};
  return app->Exec();
}
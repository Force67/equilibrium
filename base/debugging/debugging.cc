// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "debugging.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Neither musl nor Android's bionic ship <execinfo.h>/backtrace(); when
// building in the fully-static musl mode (see build/musl_static.lua) or for
// Android we stub the callstack capture instead of pulling in a backtrace
// library. Windows has neither <execinfo.h> nor <cxxabi.h>, so it takes the
// same stubbed path (a DbgHelp-based capture can replace this later).
#if defined(BASE_MUSL_STATIC) || defined(__ANDROID__) || defined(_WIN32)
#define BASE_NO_BACKTRACE
#endif

#if !defined(BASE_NO_BACKTRACE)
#include <execinfo.h>
#include <cxxabi.h>
#endif

#if defined(_WIN32)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent(void);
#endif

namespace base {

bool IsDebuggerAttached() {
#if defined(_WIN32)
  return ::IsDebuggerPresent() != 0;
#else
  FILE* f = ::fopen("/proc/self/status", "r");
  if (!f) return false;

  char line[256];
  while (::fgets(line, sizeof(line), f)) {
    if (::strncmp(line, "TracerPid:\t", 11) == 0) {
      ::fclose(f);
      return ::atoi(line + 11) != 0;
    }
  }
  ::fclose(f);
  return false;
#endif
}

#if !defined(BASE_NO_BACKTRACE)
// Try to demangle a single backtrace_symbols frame string.
// Input looks like: "./build/voxel_beta(_ZN7physics...+0x1a) [0x55...]"
// We extract the mangled name between '(' and '+' and demangle it.
static base::String DemangleFrame(const char* raw) {
  const char* lparen = ::strchr(raw, '(');
  const char* plus = lparen ? ::strchr(lparen, '+') : nullptr;

  if (!lparen || !plus || plus <= lparen + 1) {
    return base::String(raw);
  }

  // Extract mangled name.
  i32 len = static_cast<i32>(plus - lparen - 1);
  char mangled[512];
  if (len >= static_cast<i32>(sizeof(mangled))) {
    return base::String(raw);
  }
  __builtin_memcpy(mangled, lparen + 1, len);
  mangled[len] = '\0';

  // Demangle.
  int status = 0;
  char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
  if (status != 0 || !demangled) {
    return base::String(raw);
  }

  // Build a clean string: "demangled+offset"
  base::String result(demangled);
  result += base::String(plus, static_cast<i32>(::strlen(plus)));
  ::free(demangled);

  return result;
}
#endif  // !BASE_NO_BACKTRACE

base::Vector<base::String> CaptureCallstack(i32 skipFrames, i32 maxFrames) {
  base::Vector<base::String> result;

#if defined(BASE_NO_BACKTRACE)
  (void)skipFrames;
  (void)maxFrames;
  return result;
#else
  void* buffer[64];
  i32 total = maxFrames;
  if (total > 64) total = 64;

  i32 count = backtrace(buffer, total + skipFrames);
  char** symbols = backtrace_symbols(buffer, count);
  if (!symbols) return result;

  for (i32 i = skipFrames; i < count; ++i) {
    result.push_back(DemangleFrame(symbols[i]));
  }
  ::free(symbols);
  return result;
#endif
}

}  // namespace base

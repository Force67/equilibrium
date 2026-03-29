// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "debugging.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>

#include <cxxabi.h>

namespace base {

bool IsDebuggerAttached() {
  FILE* f = std::fopen("/proc/self/status", "r");
  if (!f) return false;

  char line[256];
  while (std::fgets(line, sizeof(line), f)) {
    if (std::strncmp(line, "TracerPid:\t", 11) == 0) {
      std::fclose(f);
      return std::atoi(line + 11) != 0;
    }
  }
  std::fclose(f);
  return false;
}

// Try to demangle a single backtrace_symbols frame string.
// Input looks like: "./build/voxel_beta(_ZN7physics...+0x1a) [0x55...]"
// We extract the mangled name between '(' and '+' and demangle it.
static base::String DemangleFrame(const char* raw) {
  const char* lparen = std::strchr(raw, '(');
  const char* plus = lparen ? std::strchr(lparen, '+') : nullptr;

  if (!lparen || !plus || plus <= lparen + 1) {
    return base::String(raw);
  }

  // Extract mangled name.
  i32 len = static_cast<i32>(plus - lparen - 1);
  char mangled[512];
  if (len >= static_cast<i32>(sizeof(mangled))) {
    return base::String(raw);
  }
  std::memcpy(mangled, lparen + 1, len);
  mangled[len] = '\0';

  // Demangle.
  int status = 0;
  char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
  if (status != 0 || !demangled) {
    return base::String(raw);
  }

  // Build a clean string: "demangled+offset"
  base::String result(demangled);
  result += base::String(plus, static_cast<i32>(std::strlen(plus)));
  std::free(demangled);

  return result;
}

base::Vector<base::String> CaptureCallstack(i32 skipFrames, i32 maxFrames) {
  base::Vector<base::String> result;

  void* buffer[64];
  i32 total = maxFrames;
  if (total > 64) total = 64;

  i32 count = backtrace(buffer, total + skipFrames);
  char** symbols = backtrace_symbols(buffer, count);
  if (!symbols) return result;

  for (i32 i = skipFrames; i < count; ++i) {
    result.push_back(DemangleFrame(symbols[i]));
  }
  std::free(symbols);
  return result;
}

}  // namespace base

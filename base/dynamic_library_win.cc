// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/dynamic_library.h>

namespace base {
DynamicLibrary::DynamicLibrary(const base::Path& path) {
  Load(path);
}

bool DynamicLibrary::Load(const base::Path& path) {
  handle_ = LoadLibraryW(path.c_str());
  return handle_;
}

bool DynamicLibrary::Free() {
  return FreeLibrary(static_cast<HMODULE>(handle_));
}

void* DynamicLibrary::FindSymbolPointer(const char* name) {
  return reinterpret_cast<void*>(
      GetProcAddress(static_cast<HMODULE>(handle_), name));
}
}  // namespace base
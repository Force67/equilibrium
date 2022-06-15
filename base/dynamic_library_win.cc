// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/check.h>
#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  DCHECK(!handle_, "Attempted to load an already existing library");

  if (handle_ = ::GetModuleHandleW(path.c_str())) {
    should_free_ = false;
    return false;
  }

  // user preference
  should_free_ = should_free;
  handle_ = ::LoadLibraryW(path.c_str());
  // TODO(Vince): verify checksum
  return handle_;
}

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  DCHECK(!handle_);
  should_free_ = false;
  return handle_ = ::GetModuleHandleW(path.c_str());
}

bool DynamicLibrary::Free() {
  return ::FreeLibrary(static_cast<HMODULE>(handle_));
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  // TODO: buyer moore search directly on nt image header???
  return reinterpret_cast<void*>(
      ::GetProcAddress(static_cast<HMODULE>(handle_), name));
}
}  // namespace base
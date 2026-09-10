// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/check.h>
#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  BASE_DCHECK(!handle_, "Attempted to load an already existing library");

  if (handle_ = ::GetModuleHandleW(path.c_str())) {
    should_free_ = false;
    return true;
  }

  // User preference.
  should_free_ = should_free;
  handle_ = ::LoadLibraryW(path.c_str());
  // TODO(Vince): verify checksum.
  return handle_;
}

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  BASE_DCHECK(!handle_);
  should_free_ = false;
  return handle_ = ::GetModuleHandleW(path.c_str());
}

bool DynamicLibrary::Free() {
  if (!handle_)
    return false;
  return ::FreeLibrary(static_cast<HMODULE>(handle_));
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  // TODO: consider a Boyer-Moore search directly on the NT image header.
  return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(handle_), name));
}
}  // namespace base
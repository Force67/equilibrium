// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/check.h>
#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  BASE_DCHECK(!handle_, "Attempted to load an already existing library");

  // LoadLibraryW returns the module that is already mapped and adds a
  // reference to it, so probing with GetModuleHandleW first would only produce
  // a borrowed handle that Free() must then refuse to release.
  handle_ = ::LoadLibraryW(path.c_str());
  // TODO(Vince): verify checksum.
  // User preference: whether the reference just taken is ours to give back.
  should_free_ = handle_ != nullptr && should_free;
  return loaded();
}

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  BASE_DCHECK(!handle_);
  // GetModuleHandleW borrows: the handle carries no reference, so there is
  // nothing for Free() to give back and FreeLibrary would decrement a count
  // that belongs to whoever loaded the module.
  should_free_ = false;
  handle_ = ::GetModuleHandleW(path.c_str());
  return loaded();
}

bool DynamicLibrary::Free() {
  if (!handle_ || !should_free_)
    return false;
  const bool released = ::FreeLibrary(static_cast<HMODULE>(handle_)) != FALSE;
  handle_ = nullptr;
  should_free_ = false;
  return released;
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  // TODO: consider a Boyer-Moore search directly on the NT image header.
  if (!handle_)
    return nullptr;
  return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(handle_), name));
}
}  // namespace base

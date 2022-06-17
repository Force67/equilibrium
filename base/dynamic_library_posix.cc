// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <dlfcn.h>
#include <base/check.h>
#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  DCHECK(!handle_, "Attempted to load an already existing library");

#if 0
  if (handle_ = ::GetModuleHandleW(path.c_str())) {
    should_free_ = false;
    return false;
  }
#endif

  // user preference
  should_free_ = should_free;

  // TODO(vince): RTLD_LAZY preferences in flags...
  handle_ = ::dlopen(path.c_str(), RTLD_NOW);
  return handle_;
}

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  // https://stackoverflow.com/questions/5103443/how-to-check-what-shared-libraries-are-loaded-at-run-time-for-a-given-process
  // https://source.chromium.org/chromium/chromium/src/+/main:base/native_library_posix.cc
  DCHECK(!handle_);
  should_free_ = false;

  DCHECK(false);
  return false;
}

bool DynamicLibrary::Free() {
  return ::dlclose(handle_);
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  return ::dlsym(handle_, name);
}
}  // namespace base
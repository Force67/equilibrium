// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <link.h>   // for linkmap
#include <dlfcn.h>  // for dlopen

#include <base/check.h>
#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

#include <base/strings/char_algorithms.h>
#include "strings/string_ref.h"

namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  DCHECK(!handle_, "Attempted to load an already existing library");
  DCHECK(!path.empty(), "Empty library path");

  if (LoadExisting(path))
    return true;

  // user preference
  should_free_ = should_free;

  // TODO(vince): RTLD_LAZY preferences in flags...
  handle_ = ::dlopen(path.c_str(), RTLD_NOW);
  return handle_;
}

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  DCHECK(!handle_, "Attempted to load an already existing library");
  DCHECK(!path.empty(), "Empty library path");

  should_free_ = false;

  struct Context {
    void* handle;
    const base::Path::BufferType& path_ref;
  } context{nullptr, path.path()};

  static auto callback = [](struct dl_phdr_info* info, size_t size,
                            void* user_pointer) {
    // the first entry may point to a string entry that is empty, but the pointer to
    // the empty string still may be valid, so we consider this in the check here
    if (info->dlpi_name == nullptr || info->dlpi_name[0] == '\0')
      return 0;

    // According to the man pages, dlpi_name is null terminated
    const base::StringRef ref(info->dlpi_name);
    DCHECK(ref.IsNullTerminated(),
           "dlapi_name is not null terminated according to spec.");

    auto* context = reinterpret_cast<Context*>(user_pointer);
    // the name itself follows the following format:
    // lib/x86_64-linux-gnu/libstdc++.so.6
    if (ref.find(context->path_ref.c_str(), 0, context->path_ref.length()) !=
        base::StringRef::npos) {
      context->handle = reinterpret_cast<void*>(info->dlpi_addr);
      return 0;
    }

    return 0;
  };

  ::dl_iterate_phdr(callback, &context);
  handle_ = context.handle;

  return loaded();
}

bool DynamicLibrary::Free() {
  return ::dlclose(handle_);
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  return ::dlsym(handle_, name);
}
}  // namespace base
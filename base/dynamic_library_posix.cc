// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// macOS is Mach-O and ships no <link.h>/dl_iterate_phdr; the already-loaded
// lookup goes through dlopen(RTLD_NOLOAD) below instead.
#if !defined(__APPLE__)
#include <link.h>  // for dl_iterate_phdr / dl_phdr_info
#endif
#include <dlfcn.h>  // for dlopen

#include "check.h"
#include "filesystem/path.h"
#include "dynamic_library.h"

#include "text/code_point_validation.h"
#include "strings/char_algorithms.h"
#include "strings/string_ref.h"
namespace base {
bool DynamicLibrary::Load(const base::Path& path, bool should_free) {
  BASE_DCHECK(!handle_, "Attempted to load an already existing library");
  BASE_DCHECK(!path.empty(), "Empty library path");

  BASE_BUGCHECK(base::DoIsStringUTF8(path.c_str(), path.length()),
           "DynamicLibrary::Load(): BASE requires paths to be utf8 encoded!");

  // An already-mapped library goes through LoadExisting, which matches partial
  // sonames ("libstdc++.so" against "libstdc++.so.6") that dlopen rejects.
  // Both paths end up holding a reference of their own, so |should_free| means
  // the same thing on either: give it back, or leave it outstanding. Giving it
  // back cannot unload a module that was already mapped, because whoever
  // mapped it still holds theirs.
  if (LoadExisting(path)) {
    should_free_ = should_free;
    return true;
  }

  // TODO(vince): RTLD_LAZY preferences in flags...
  handle_ = ::dlopen(reinterpret_cast<const char*>(path.c_str()), RTLD_NOW);
  // user preference: whether the reference just taken is ours to give back
  should_free_ = handle_ != nullptr && should_free;
  return loaded();
}

#if defined(__APPLE__)

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  BASE_DCHECK(!handle_, "Attempted to load an already existing library");
  BASE_DCHECK(!path.empty(), "Empty library path");

  // RTLD_NOLOAD returns a usable handle only when the library is already mapped
  // into the process, which is what dl_iterate_phdr emulates on Linux.
  handle_ = ::dlopen(reinterpret_cast<const char*>(path.c_str()),
                     RTLD_NOW | RTLD_NOLOAD);
  // The reference RTLD_NOLOAD took is ours; giving it back cannot unload the
  // module, because the code that loaded it still holds its own.
  should_free_ = handle_ != nullptr;
  return loaded();
}

#else

bool DynamicLibrary::LoadExisting(const base::Path& path) {
  BASE_DCHECK(!handle_, "Attempted to load an already existing library");
  BASE_DCHECK(!path.empty(), "Empty library path");

  // dl_iterate_phdr matches partial names -- "libc.so" against
  // "/lib/x86_64-linux-gnu/libc.so.6" -- which dlopen does not, so the walk
  // runs first and reports only the name it matched. Its dlpi_addr is the
  // module's load bias, not a handle: dlsym and dlclose reject it.
  struct Context {
    const char* name;
    const base::Path::BufferType& path_ref;
  } context{nullptr, path.path()};

  static auto callback = [](struct dl_phdr_info* info, size_t size, void* user_pointer) {
    // the first entry may point to a string entry that is empty, but the
    // pointer to the empty string still may be valid, so we consider this in
    // the check here
    if (info->dlpi_name == nullptr || info->dlpi_name[0] == '\0')
      return 0;

    // According to the man pages, dlpi_name is null terminated
    const base::StringRefU8 ref(reinterpret_cast<const char8_t*>(info->dlpi_name));

    BASE_BUGCHECK(base::DoIsStringUTF8(ref.c_str(), ref.length()),
             "DynamicLibrary::LoadExisting(): BASE requires paths to be utf8 "
             "encoded!");

    // DCHECK(ref.IsNullTerminated(),
    //        "dlapi_name is not null terminated according to spec.");

    auto* context = reinterpret_cast<Context*>(user_pointer);
    // the name itself follows the following format:
    // lib/x86_64-linux-gnu/libstdc++.so.6
    if (ref.find(context->path_ref.c_str(), 0, context->path_ref.length()) !=
        base::StringRef::npos) {
      context->name = info->dlpi_name;
      return 1;  // stop the walk on the first match
    }

    return 0;
  };

  ::dl_iterate_phdr(callback, &context);
  if (!context.name)
    return false;

  // RTLD_NOLOAD turns the matched name into a handle without loading anything.
  // The reference it takes is ours; giving it back cannot unload the module,
  // because the code that loaded it still holds its own.
  handle_ = ::dlopen(context.name, RTLD_NOW | RTLD_NOLOAD);
  should_free_ = handle_ != nullptr;
  return loaded();
}

#endif  // __APPLE__

bool DynamicLibrary::Free() {
  if (!handle_ || !should_free_)
    return false;
  const bool released = ::dlclose(handle_) == 0;
  handle_ = nullptr;
  should_free_ = false;
  return released;
}

void* DynamicLibrary::FindSymbolPointer(const char* name) const {
  // dlsym has no defined behaviour for a null handle; RTLD_DEFAULT is a
  // distinct sentinel, not the null pointer.
  return handle_ ? ::dlsym(handle_, name) : nullptr;
}
}  // namespace base

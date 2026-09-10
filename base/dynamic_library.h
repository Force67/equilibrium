// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>

namespace base {

class Path;

class BASE_EXPORT DynamicLibrary {
 public:
  DynamicLibrary() = default;
  DynamicLibrary(DynamicLibrary&&) noexcept;

  DynamicLibrary(const base::Path& path) { Load(path); }
  ~DynamicLibrary();

  // Loads the library, reusing the existing mapping when it is already loaded.
  // Pass |should_free| to have this object give back the reference the load
  // acquires; without it the reference outlives the object.
  bool Load(const base::Path&, bool should_free = false);

  // Obtains a handle for a library that is already in the process, without
  // taking over its lifetime. The module stays mapped for whoever loaded it.
  bool LoadExisting(const base::Path&);

  // Hands |handle_| back to the loader and invalidates it. Returns false when
  // this object holds no reference to give back -- a borrowed handle carries
  // none, and releasing one would unload a module this object does not own.
  bool Free();

  void* FindSymbolPointer(const char* symbol_name) const;

  // fetch an exported symbol, you can also pass an ordinal index on windows by
  // casting to const char*
  template <typename T>
  T FindSymbol(const char* symbol_name) const {
    return reinterpret_cast<T>(FindSymbolPointer(symbol_name));
  }

  bool loaded() const { return handle_; }

 private:
  // Always a handle the platform's symbol lookup accepts: a dlopen result on
  // POSIX, an HMODULE on Windows. Never a link-map address.
  void* handle_{nullptr};
  // Whether |handle_| carries a loader reference that this object must return.
  // Load takes one; LoadExisting takes one on POSIX, where dlsym needs a real
  // dlopen handle, and none on Windows, where GetModuleHandle only borrows.
  bool should_free_{false};
};
}  // namespace base
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

  // This will check if the module is loaded already, and if so, just fetch the
  // loaded handle
  bool Load(const base::Path&, bool should_free = false);

  // On windows, this issues a getmodulehandle call
  bool LoadExisting(const base::Path&);

  // unloads the library, if should_free_ is true
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
  void* handle_{nullptr};
  bool should_free_{false};
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/filesystem/path.h>

namespace base {

class BASE_EXPORT DynamicLibrary {
 public:
  DynamicLibrary() = default;

  template <bool TExpectLoaded = false>
  DynamicLibrary(const base::Path& path) {
    if constexpr (TExpectLoaded) {
      LoadExisting(path);
    } else
      Load(path);
  }

  DynamicLibrary(DynamicLibrary&&) noexcept;

  ~DynamicLibrary();

  bool Load(const base::Path&, bool should_free = false);

  // On windows, this issues a getmodulehandle call
  // however, be careful to have a refcount >1, or we might free the library on
  // destruction
  bool LoadExisting(const base::Path&);

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
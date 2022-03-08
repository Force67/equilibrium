// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/filesystem/path.h>

namespace base {

class BASE_EXPORT DynamicLibrary {
 public:
  DynamicLibrary() = default;
  DynamicLibrary(const base::Path&);

  bool Load(const base::Path&);
  bool Free();

  void* FindSymbolPointer(const char* symbol_name);

  template<typename T>
  T FindSymbol(const char* symbol_name) {
    return reinterpret_cast<T>(FindSymbolPointer(symbol_name));
  }

  bool loaded() const { return handle_; }

 private:
  void* handle_{nullptr};
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/cxx_lifetime.h>

namespace base {
template <typename T>
struct DefaultDeleter {
  static void Delete(T* raw_pointer) {
    if constexpr (base::IsArray<T>)
      delete[] raw_pointer;
    else
      delete raw_pointer;
  }
};
}  // namespace base
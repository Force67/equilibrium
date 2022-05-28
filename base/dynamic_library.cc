// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/dynamic_library.h>

namespace base {
DynamicLibrary::DynamicLibrary(DynamicLibrary&& rhs) noexcept {
  handle_ = rhs.handle_;
  should_free_ = rhs.should_free_;
  // invalidate the previous instance
  rhs.handle_ = nullptr;
}

DynamicLibrary::~DynamicLibrary() {
  if (should_free_)
    DCHECK(Free());
}
}  // namespace base
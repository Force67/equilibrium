// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/dynamic_library.h>

namespace base {
DynamicLibrary::DynamicLibrary(DynamicLibrary&& rhs) noexcept {
  handle_ = rhs.handle_;
  should_free_ = rhs.should_free_;
  // invalidate the previous instance
  rhs.handle_ = nullptr;
  rhs.should_free_ = false;
}

DynamicLibrary::~DynamicLibrary() {
  if (!should_free_)
    return;
  // Free() cannot sit inside the DCHECK expression: BASE_DCHECK compiles its
  // argument away in shipping builds, which would skip the release entirely.
  const bool released = DynamicLibrary::Free();
  BASE_DCHECK(released, "Failed to release loaded library");
  TK_UNUSED(released);
}
}  // namespace base
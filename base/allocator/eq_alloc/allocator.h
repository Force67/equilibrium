// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>

namespace base {

class PageTable;

struct Allocator {
  friend struct MemoryCoordinator;

 protected:
  // with the hopes that the compiler will devirutalize this.
  virtual void* Allocate(PageTable&, size_t size, size_t alignment = 1024) {
    IMPOSSIBLE;
    return nullptr;
  }
  virtual void Free(void* block) { IMPOSSIBLE; }
};
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>

namespace base {

struct Allocator {
  friend struct MemoryCoordinator;
 protected:
  // with the hopes that the compiler will devirutalize this.
  constexpr virtual void* Allocate(size_t size, size_t alignment = 1024) {
    DEBUG_TRAP;
  }
  constexpr virtual void Free(void* block) { DEBUG_TRAP; }
};

struct MemoryCoordinator {
  void* Allocate(size_t size);
  void Free(void*);

 private:
  Allocator* allocators_[3]{};
};

STRONG_INLINE MemoryCoordinator& memory_coordinator();

// todo: this should capture the context better.
using OutOfMemoryHandler = void();
void SetOutOfMemoryHandler(OutOfMemoryHandler*);
void InvokeOutOfMemoryHandler();
}  // namespace base

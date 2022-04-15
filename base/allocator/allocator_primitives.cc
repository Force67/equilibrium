// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
// Alternative name: memory_primitives.

#include <malloc.h>
#include <base/allocator/memory_coordinator.h>

namespace base {
void* Allocate(size_t size) {
  return memory_coordinator().Allocate(size);
}

void Free(void* block) {
  memory_coordinator().Free(block);
}
}  // namespace base
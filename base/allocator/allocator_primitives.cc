// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
// Alternative name: memory_primitives.

#include <malloc.h>
#include <base/allocator/allocator_primitives.h>

namespace base {
void* Allocate(size_t size) {
  return malloc(size);
}

void Free(void* block) {
  free(block);
}
}  // namespace base
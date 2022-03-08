// Copyright (C) 2021 Force67 <github.com/Force67>.
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
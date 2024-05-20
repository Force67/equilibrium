// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/heap_allocator.h>

// TODO: redzone allocator.
namespace base {
void* HeapAllocator::Allocate(mem_size size, mem_size) {
  return nullptr;
}

void* HeapAllocator::ReAllocate(void* former_block, mem_size new_size, mem_size) {
  return nullptr;
}

mem_size HeapAllocator::Free(void* block) {
  return 0u;
}

mem_size HeapAllocator::QueryAllocationSize(void* block) {
  return 0u;
}
}  // namespace base
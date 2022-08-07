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

bool HeapAllocator::Free(void* block) {
  return false;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/allocator_primitives.h>

namespace base::allocator_primitives {

void* Allocate(mem_size size) {
  return memory_coordinator().Allocate(size);
}

void* ReAllocate(void* former, mem_size new_size) {
  return memory_coordinator().ReAllocate(former, new_size);
}

void Free(void* block) {
  if (block != nullptr)
    memory_coordinator().Free(block);
}

void* AllocateTracked(mem_size size, const base::SourceLocation& /*source_loc*/) {
  return memory_coordinator().Allocate(size);
}

void FreeTracked(void* block, const base::SourceLocation& /*source_loc*/) {
  memory_coordinator().Free(block);
}

namespace v2 {
MemoryBlock Allocate(mem_size size, AlignmentValue alignment) {
  return memory_coordinator().AllocateAligned(size, alignment);
}

MemoryBlock TryExpand(MemoryBlock block, mem_size new_size, AlignmentValue alignment) {
  return memory_coordinator().ReAllocateAligned(block, new_size, alignment);
}

bool Deallocate(MemoryBlock block, AlignmentValue alignment) {
  return memory_coordinator().Deallocate(block, alignment);
}
}  // namespace v2

namespace v3 {
MemoryBlockV3 Allocate(mem_size size,
                       AlignAndSkew /*align_and_skew*/,
                       AllocationFlags flags,
                       AllocationHints hints) {
  void* ptr = memory_coordinator().Allocate(size);
  return MemoryBlockV3{
      .pointer = ptr, .size = size, .tracking = {.flags = flags, .hints = hints}};
}

bool Deallocate(MemoryBlockV3 block_info, AlignAndSkew /*align_and_skew*/) {
  if (block_info.pointer)
    memory_coordinator().Free(block_info.pointer);
  return true;
}
}  // namespace v3
}  // namespace base::allocator_primitives

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Basic Memory primitives, like the usual c malloc derivatives.

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/allocator_primitives.h>
#include <base/allocator/allocator_symbol_override.in>

namespace base::allocator_primitives {

// (implicit v1 namespace)
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

void* AllocateTracked(mem_size size, const base::SourceLocation& source_loc) {
  auto& mc = memory_coordinator();

  void* block = mc.Allocate(size);
  // mc.tracker();
  return block;
}

void FreeTracked(void* block, const base::SourceLocation& source_loc) {
  auto& mc = memory_coordinator();
  mc.Free(block);

  // mc.tracker();
}

// 2x api
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

// 3x api
namespace v3 {

MemoryBlockV3 Allocate(mem_size size,
                       AlignAndSkew align_and_skew,
                       AllocationFlags flags,
                       AllocationHints hints) {
  static_assert(sizeof(AllocationFlags) == (sizeof(MemoryBlockV3::tracking_flags) / 2),
                "AllocationFlags are misaligned");
  static_assert(sizeof(AllocationHints) == (sizeof(MemoryBlockV3::tracking_flags) / 2),
                "AllocationHints are misaligned");

  // TODO: allocate

  return MemoryBlockV3{
      .pointer = nullptr, .size = 0, .tracking = {.flags = flags, .hints = hints}};
}

bool Deallocate(MemoryBlockV3 block_info, AlignAndSkew align_and_skew) {
  return false;
}
}  // namespace v3
}  // namespace base::allocator_primitives
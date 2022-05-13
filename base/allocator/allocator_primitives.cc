// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
// Alternative name: memory_primitives.

#include <malloc.h>
#include <base/allocator/memory_coordinator.h>
#include "allocator_primitives.h"

namespace base {
void* Allocate(size_t size) {
  return memory_coordinator().Allocate(size);
}

void Free(void* block) {
  memory_coordinator().Free(block);
}

void* AllocateTracked(size_t size, const base::SourceLocation&) {
  auto& mc = memory_coordinator();

  void* block = mc.Allocate(size);
  //mc.tracker();
  return block;
}

void FreeTracked(void* block, const base::SourceLocation&) {
  auto& mc = memory_coordinator();
  mc.Free(block);

  //mc.tracker();
}

}  // namespace base
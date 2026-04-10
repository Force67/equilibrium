// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <mimalloc.h>
#include <base/arch.h>

namespace base {

// Mimalloc-backed router for the memory coordinator.
// Unlike DefaultCRTRouter, Free() returns the actual freed size via
// mi_usable_size() so the memory tracker can accurately decrement.
class MimallocRouter {
 public:
  void* Allocate(mem_size size) { return mi_malloc(size); }

  void* AllocateAligned(mem_size size, mem_size alignment) {
    return mi_malloc_aligned(size, alignment);
  }

  void* ReAllocate(void* former, mem_size new_size, pointer_diff& diff_out) {
    mem_size old_size = former ? mi_usable_size(former) : 0;
    void* block = mi_realloc(former, new_size);
    diff_out = pointer_diff(new_size) - pointer_diff(old_size);
    return block;
  }

  void* ReAllocateAligned(void* former_block,
                          mem_size former_size,
                          mem_size new_size,
                          mem_size alignment) {
    (void)former_size;
    return mi_realloc_aligned(former_block, new_size, alignment);
  }

  mem_size Free(void* block) {
    if (!block) return 0;
    mem_size size = mi_usable_size(block);
    mi_free(block);
    return size;
  }

  bool Deallocate(void* block, mem_size size, mem_size alignment) {
    (void)size;
    (void)alignment;
    mi_free_aligned(block, alignment);
    return true;
  }
};
}  // namespace base

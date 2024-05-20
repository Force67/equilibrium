// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>

namespace base {

class PageTable;

class Allocator {
 public:
  Allocator() = default;

  virtual ~Allocator() = default;

  // with the hopes that the compiler will devirutalize this.
  virtual void* Allocate(mem_size size, mem_size user_alignment = 1024) {
    (void)size;
    (void)user_alignment;
    IMPOSSIBLE;
    return nullptr;
  }

  // re-allocate a block of memory.
  // e.g if the block is already allocated, it will be copied to a new location.
  // if the block is not allocated, it will be allocated.
  virtual void* ReAllocate(void* former_block,
                           mem_size new_size,
                           mem_size user_alignment = 1024) {
    (void)former_block;
    (void)new_size;
    (void)user_alignment;
    IMPOSSIBLE;
    return nullptr;
  }

  // returns the amount of memory that was freed.
  virtual mem_size Free(void* block) {
    (void)block;
    IMPOSSIBLE;
    return 0u;
  }

  // returns the amount of memory that is allocated by the block.
  virtual mem_size QueryAllocationSize(void* block) {
	(void)block;
	IMPOSSIBLE;
	return 0u;
  }
};
}  // namespace base

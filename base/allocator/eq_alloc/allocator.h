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
  virtual void* Allocate(mem_size, mem_size user_alignment = 1024) {
    IMPOSSIBLE;
    return nullptr;
  }

  virtual void* ReAllocate(void* former_block,
                           mem_size new_size,
                           mem_size user_alignment = 1024) {
    IMPOSSIBLE;
    return nullptr;
  }

  virtual bool Free(void* block) {
    IMPOSSIBLE;
    return false;
  }
};
}  // namespace base

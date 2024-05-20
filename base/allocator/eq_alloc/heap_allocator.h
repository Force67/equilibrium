// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/eq_alloc/allocator.h>

namespace base {
class HeapAllocator final : public Allocator {
 public:
  void* Allocate(mem_size, mem_size user_alignment = 1024) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 1024) override;

  mem_size Free(void* block) override;

  mem_size QueryAllocationSize(void* block) override;
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/eq_alloc/allocator.h>

namespace base {
class PageAllocator final : public Allocator {
 public:
  inline PageAllocator(PageTable& t) : page_table_(t) {}

  void* Allocate(mem_size, mem_size user_alignment = 1024) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 1024) override;

  bool Free(void* block) override;

 private:
  PageTable& page_table_;
};
}  // namespace base
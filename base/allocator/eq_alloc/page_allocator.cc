// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/page_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

namespace base {
void* PageAllocator::Allocate(mem_size size, mem_size) {
  DCHECK(size <= base::eq_allocation_constants::kPageThreshold,
         "Size limit for page allocation exceeded");

  return page_table_.RequestPage(base::PageProtectionFlags::RW, nullptr);
}

void* PageAllocator::ReAllocate(void* former_block, mem_size new_size, mem_size) {
  if (new_size <= base::eq_allocation_constants::kPageThreshold) {
    // since every page is 64k, there is still space in the page
    return former_block;
  } else
    return nullptr;
}

mem_size PageAllocator::Free(void* block) {
  return page_table_.ReleasePage(block);
}

mem_size PageAllocator::QueryAllocationSize(void* block) {
  (void)block;
  return base::eq_allocation_constants::kPageSize;
}
}  // namespace base
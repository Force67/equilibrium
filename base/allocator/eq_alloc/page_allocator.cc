// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/page_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/threading/lock_guard.h>

namespace base {

void PageAllocator::PushFreePage(void* page) {
  // store the current head in the page itself (intrusive stack)
  *reinterpret_cast<void**>(page) = free_stack_;
  free_stack_ = page;
}

void* PageAllocator::PopFreePage() {
  if (!free_stack_)
    return nullptr;
  void* page = free_stack_;
  free_stack_ = *reinterpret_cast<void**>(page);
  return page;
}

void* PageAllocator::Allocate(mem_size size, mem_size) {
  BASE_DCHECK(size <= base::eq_allocation_constants::kPageThreshold,
         "Size limit for page allocation exceeded");

  base::NonOwningScopedLockGuard _(lock_);
  (void)_;

  // try cached page first (avoids mmap syscall)
  if (void* cached = PopFreePage())
    return cached;

  return page_table_.RequestPage(base::PageProtectionFlags::RW, nullptr);
}

void* PageAllocator::ReAllocate(void* former_block, mem_size new_size, mem_size) {
  if (new_size <= base::eq_allocation_constants::kPageThreshold) {
    // still fits in a single page
    return former_block;
  }
  return nullptr;
}

mem_size PageAllocator::Free(void* block) {
  base::NonOwningScopedLockGuard _(lock_);
  (void)_;

  // cache the page for reuse instead of returning to the kernel
  PushFreePage(block);
  return base::eq_allocation_constants::kPageSize;
}

mem_size PageAllocator::QueryAllocationSize(void* block) {
  (void)block;
  return base::eq_allocation_constants::kPageSize;
}
}  // namespace base

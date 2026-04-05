// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/eq_alloc/allocator.h>
#include <base/threading/spinning_mutex.h>

namespace base {

// Allocates whole 64 KiB pages for medium-sized requests (1-64 KiB).
// Freed pages are cached in a lock-free stack to avoid mmap/munmap syscalls
// on the hot path.
class PageAllocator final : public Allocator {
 public:
  inline PageAllocator(PageTable& t) : page_table_(t) {}

  void* Allocate(mem_size, mem_size user_alignment = 1024) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 1024) override;

  mem_size Free(void* block) override;

  mem_size QueryAllocationSize(void* block) override;

 private:
  // intrusive free-page stack: the first pointer_size bytes of a freed page
  // store the pointer to the next free page.
  void* PopFreePage();
  void PushFreePage(void* page);

  PageTable& page_table_;
  base::SpinningMutex lock_;
  void* free_stack_{nullptr};
};
}  // namespace base

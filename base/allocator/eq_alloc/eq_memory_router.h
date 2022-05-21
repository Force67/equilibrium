// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// This defines the memory routing stategy used by eq_alloc
#pragma once
#include <base/compiler.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

namespace base {
struct EQMemoryRouter {
  // allocate and route the memory here, preferably at compile time
  // if < kBucketThreshold -> Invoke bucket allocator
  // if < PageTreshhold -> Invoke page table
  // else: Directly reserve heap memory, which uses an intrusive list of free blocks
  // red black tree. like dmalloc.
  STRONG_INLINE void* Allocate(const size_t size) {
    auto& pages = *page_table();
    // branchless solution?
    if (size <= eq_allocation_constants::kBucketThreshold)
      return allocators_[0]->Allocate(pages, size,
                                      eq_allocation_constants::kBucketThreshold);
    if (size <= eq_allocation_constants::kPageThreshold)
      return pages.RequestPage();

    // directly allocate object on the heap
    return allocators_[2]->Allocate(pages, 2 /*TODO: align to the power of two!*/);
  }

  STRONG_INLINE void* ReAllocate(void* former, size_t new_size) {
    DEBUG_TRAP;

    return nullptr;
  }

  STRONG_INLINE void Free(void* block) {
    auto& pages = *page_table();
    for (auto* a : allocators_) {
      if (!a)
        continue;

      // TODO: consult the page mapping
      // a->
    }
  }

 private:
  void InitializeAllocators(PageTable*);

  PageTable* page_table();

 private:
  // TODO: figure out a way to run the dtor?
  alignas(PageTable) byte page_table_data_[sizeof(PageTable)]{};
  Allocator* allocators_[4]{nullptr};
};

}  // namespace base
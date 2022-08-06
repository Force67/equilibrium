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
  // TODO: check allocation preference here.?
  STRONG_INLINE void* Allocate(const mem_size size) {
    // allocate and route the memory here, preferably at compile time
    // if < kBucketThreshold -> Invoke bucket allocator
    // if < PageTreshhold -> Invoke page table
    // else: Directly reserve heap memory, which uses an intrusive list of free
    // blocks red black tree. like dmalloc.
    auto& pages = *page_table();
    // branchless solution?
    if (size <= eq_allocation_constants::kBucketThreshold)
      return allocators_[0]->Allocate(pages, size,
                                      eq_allocation_constants::kBucketThreshold);

    if (size <= eq_allocation_constants::kPageThreshold)
      return pages.RequestPage(base::PageProtectionFlags::RW);

    // directly allocate object on the heap
    return allocators_[2]->Allocate(pages, 2 /*TODO: align to the power of two!*/);
  }

  STRONG_INLINE void* ReAllocate(void* former,
                                 const mem_size new_size,
                                 pointer_diff& diff_out) {
    auto& page_tab = *page_table();
    diff_out = new_size - block_size(page_tab, former);

    auto* allocator = FindOwningAllocator(former);
    DCHECK(allocator, "ReAllocate(): Orphaned memory?");

    // we try our hardest to grow in place.
    return allocator->ReAllocate(page_tab, former, new_size);
  }

  STRONG_INLINE mem_size Free(void* block) {
    auto& page_tab = *page_table();
    const mem_size bytes_freed{block_size(page_tab, block)};

    auto* allocator = FindOwningAllocator(block);
    DCHECK(allocator, "Free(): Orphaned memory?");

    allocator->Free(block);
    return bytes_freed;
  }

 private:
  mem_size block_size(PageTable& tab, void* block) { return 0; }

  Allocator* FindOwningAllocator(void* block) {
    // we should already know the index, since everything is aligned to a 1mib
    // boundary, so we can assume the page alignment, so therefore the offset of the
    // block. (starting from page_base)?
    // and since we are the god father allocator we could hopefully grow that way?

    for (auto i = 0; i < sizeof(allocator_mapping_table_) / sizeof(u8); i++) {
      if (allocator_mapping_table_[i]) {
      }
    }

    return nullptr;
  }

  void InitializeAllocators(PageTable*);

  PageTable* page_table();

 private:
  // TODO: figure out a way to run the dtor of the pagetable?
  alignas(PageTable) byte page_table_data_[sizeof(PageTable)]{};
  Allocator* allocators_[256]{nullptr};
  // see
  // https://cdn.discordapp.com/attachments/818575873203503165/1005495331636662373/unknown.png
  // every index (number stored from 1 - 255 at given position) corresponds to a page
  // at offset n
  u8 allocator_mapping_table_[1_tib >> 20]{0};
};

}  // namespace base
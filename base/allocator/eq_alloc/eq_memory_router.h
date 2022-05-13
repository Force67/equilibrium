// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// This defines the memory routing stategy used by eq_alloc
#pragma once

#include <base/compiler.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/allocation_constants.h>

namespace base {
struct EQMemoryRouter {
  // allocate and route the memory here, preferably at compile time
  // if < kBucketThreshold -> Invoke bucket allocator
  // if < PageTreshhold -> Invoke page table
  // else: Directly reserve heap memory, which uses an intrusive list of free blocks
  // red black tree. like dmalloc.

  // take the fast path if its a concrete object that we are allocating
  template <typename T>
  STRONG_INLINE constexpr T* Allocate() {
    constexpr auto object_size = sizeof(T);

    auto& pt = *page_table();
    if constexpr (object_size <= allocation_constants::kBucketThreshold)
      return allocators_[0]->Allocate(pt, object_size,
                                      allocation_constants::kBucketThreshold);

    if constexpr (object_size <= allocation_constants::kPageThreshold)
      return page_table_.Allocate(nullptr, 1337, 0, false, false);

    return allocators_[2]->Allocate(page_table_,
                                    2 /*TODO: align to the power of two!*/);
  }

  STRONG_INLINE constexpr void* Allocate(const size_t size) {
#if 0
    if constexpr (size <= kBucketThreshold)
      return allocators_[0]->Allocate(page_table_, size, kBucketThreshold);
    if (size <= kPageThreshold)
      return page_table_.Allocate(nullptr, 1337, 0, false, false);
    return allocators_[2]->Allocate(page_table_,
                                    2 /*TODO: align to the power of two!*/);
#endif
    return nullptr;
  }

  PageTable* page_table() {
    if (!page_table_data_[0])
      return new (page_table_data_) PageTable;
    return reinterpret_cast<PageTable*>(&page_table_data_[0]);
  }

 private:
  // TODO: figure out a way to run the dtor?
  alignas(PageTable) byte page_table_data_[sizeof(PageTable)]{};
  Allocator* allocators_[4]{nullptr};
};

}  // namespace base
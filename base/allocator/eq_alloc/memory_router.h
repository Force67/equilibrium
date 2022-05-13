// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// This defines the memory routing stategy used by eq_alloc
#pragma once

namespace base {
struct EQIRouter {
  // allocate and route the memory here, preferably at compile time
  // if < kBucketThreshold -> Invoke bucket allocator
  // if < PageTreshhold -> Invoke page table
  // else: Directly reserve heap memory, which uses an intrusive list of free blocks
  // red
  //       black tree. like dmalloc.

};


}  // namespace base

#if 0

  // take the fast path if its a concrete object that we are heap allocating
  template <typename T>
  STRONG_INLINE constexpr T* Allocate() {
    constexpr auto size = sizeof(T);
    if constexpr (size <= kBucketThreshold)
      return allocators_[0]->Allocate(page_table_, size, kBucketThreshold);
    if constexpr (size <= kPageThreshold)
      return page_table_.Allocate(nullptr, 1337, 0, false, false);
    return allocators_[2]->Allocate(page_table_,
                                    2 /*TODO: align to the power of two!*/);
  }

  STRONG_INLINE constexpr void* Allocate(const size_t size) {
    if constexpr (size <= kBucketThreshold)
      return allocators_[0]->Allocate(page_table_, size, kBucketThreshold);
    if (size <= kPageThreshold)
      return page_table_.Allocate(nullptr, 1337, 0, false, false);
    return allocators_[2]->Allocate(page_table_,
                                    2 /*TODO: align to the power of two!*/);
  }
#endif
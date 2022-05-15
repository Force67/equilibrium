// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>

namespace base {
void* BucketAllocator::Allocate(PageTable& pages, size_t size, size_t alignment) {
  // log2 based alignment
  __debugbreak();
  Bucket* my_bucket_ = FindFreeBucket();
  if (!my_bucket_) {
    void* page_memory = pages.RequestPage();
    __debugbreak();
  }

  return nullptr;
}
void BucketAllocator::Free(void* block) {}

BucketAllocator::Bucket* BucketAllocator::FindFreeBucket() {
    // lock
  return nullptr;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/bucket_allocator.h>

namespace base {
void* BucketAllocator::Allocate(PageTable& pages, size_t size, size_t alignment) {
  return nullptr;
}
void BucketAllocator::Free(void* block) {}
}  // namespace base
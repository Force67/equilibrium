// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/bucket_allocator.h>

namespace base {
constexpr void* BucketAllocator::Allocate(size_t size, size_t alignment) {
  return nullptr;
}
constexpr void BucketAllocator::Free(void* block) {

}
}  // namespace base
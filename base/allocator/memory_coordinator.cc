// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
//
// This implementation is larged based on the brilliant gdc talk by the folks at
// bluepoint studios. https://www.youtube.com/watch?v=fcBZEZWGYek

#include <base/allocator/memory_coordinator.h>

namespace base {

namespace {
constexpr size_t kBucketThreshold = 1024;
constexpr size_t kPageThreshold = 1024 * 64;

}  // namespace

MemoryCoordinator::MemoryCoordinator() {}

void* MemoryCoordinator::Allocate(size_t size) {
  if (size <= kBucketThreshold) {
    return allocators_[0]->Allocate(size, kBucketThreshold);
  }
  if (size <= kPageThreshold) {
    return allocators_[1]->Allocate(size, kPageThreshold);
  }

  // we use the heap allocator, which uses an intrusive list of free blocks red
  // black tree. like dmalloc.
  return allocators_[2]->Allocate(size, 2 /*TODO: align to the power of two!*/);
}

void MemoryCoordinator::Free(void* block) {
  // Now we need to see the owner.
}
}  // namespace base
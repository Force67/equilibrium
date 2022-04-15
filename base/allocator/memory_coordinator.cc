// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// This implementation is larged based on the brilliant gdc talk by the folks at
// bluepoint studios. https://www.youtube.com/watch?v=fcBZEZWGYek

#include <base/allocator/size_literals.h>
#include <base/allocator/memory_coordinator.h>

namespace base {

namespace {
using namespace base::size_literals;

constexpr size_t kBucketThreshold = 1_kib;
constexpr size_t kPageThreshold = 64_kib;

// no constructor desired
static constinit MemoryCoordinator MemoryRouter{};

static OutOfMemoryHandler* OOMHandler{nullptr};
}  // namespace

// TODO: constexpr size opt
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

MemoryCoordinator& memory_coordinator() {
  return MemoryRouter;
}

void SetOutOfMemoryHandler(OutOfMemoryHandler* new_handler) {
  OOMHandler = new_handler;
}

void InvokeOutOfMemoryHandler() {
  // give redzone memory (a prereserved tiny segment for throwing the error.)
  if (!OOMHandler)
  // assert
  {
  }

  OOMHandler();
}
}  // namespace base
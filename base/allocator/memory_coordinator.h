// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#endif

namespace base {
struct DefaultRouter {
  void* Allocate(size_t size) { return ::malloc(size); }
  void* ReAllocate(void* former, size_t new_size) {
    return ::realloc(former, new_size);
  }
  void Free(void* block) { ::free(block); }
};

// this struct musn't instantiate any complex objects
template <class TRouter>
struct MCInstance {
  // goal is to have these folded in the ::new/alloc operators
  STRONG_INLINE void* Allocate(size_t size) {
    tracker_.TrackAllocation(size);
    return router_.Allocate(size);
  }

  STRONG_INLINE void* ReAllocate(void* former, size_t new_size) {
    // TODO: fix the tracker...
    tracker_.TrackAllocation(new_size);
    return router_.ReAllocate(former, new_size);
  }

  STRONG_INLINE void Free(void* address) {
    // TODO: determine the size freed
    tracker_.TrackDeallocation(0);
    router_.Free(address);
  }

  auto& tracker() { return tracker_; }

 private:
  MemoryTracker tracker_;
  TRouter router_;
};
// this configures the use of different allocators
#if (BASE_USE_EQ_ALLOCATOR)
using MemoryCoordinator = MCInstance<EQMemoryRouter>;
#else
using MemoryCoordinator = MCInstance<DefaultRouter>;
#endif

// get the global instance
// TODO: current this isnt folded..
MemoryCoordinator& memory_coordinator();

using OutOfMemoryHandler = void(MemoryCoordinator&, void*);
void SetOutOfMemoryHandler(OutOfMemoryHandler*, void* user_context = nullptr);
void InvokeOutOfMemoryHandler();

// TBD: capture line, file, etc
#define BASE_INVOKE_OOM
}  // namespace base

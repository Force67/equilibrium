// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#else
#include <base/allocator/default_crt_alloc.h>
#endif

namespace base {
// this struct musn't instantiate any complex objects
template <class TRouter>
struct MCInstance {
  // goal is to have these folded in the ::new/alloc operators
  STRONG_INLINE void* Allocate(size_t size) {
    tracker_.TrackAllocation(size);
    return router_.Allocate(size);
  }

  STRONG_INLINE void* ReAllocate(void* former, size_t new_size) {
    // this stuff sucks..
    mem_size diff_out = 0;
    void* block = router_.ReAllocate(former, new_size, diff_out);
    // TODO: fix the tracker... those track allocations could be all one function
    // as we can simply negate the sign.
    tracker_.TrackAllocation(diff_out);
    return block;
  }

  STRONG_INLINE void Free(void* address) {
    const mem_size amount_freed = router_.Free(address);
    tracker_.TrackDeallocation(amount_freed);
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
}  // namespace base

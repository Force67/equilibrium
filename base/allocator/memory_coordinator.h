// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/profiling.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#else
#include <base/allocator/default_crt_alloc.h>
#endif

namespace base {

// do not instantiate any complex routers.
template <class TRouter>
struct MCInstance {
  // goal is to have these folded in the ::new/alloc operators

  inline void* Allocate(size_t size) {
    void* block = router_.Allocate(size);
    tracker_.TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return block;
  }

  inline void* ReAllocate(void* former, size_t new_size) {
    pointer_diff diff_out = 0; /*already signed*/
    void* block = router_.ReAllocate(former, new_size, diff_out);
    tracker_.TrackOperation(block, diff_out);
    return block;
  }

  inline void Free(void* address) {
    BASE_PROFILE_FREE(address);
    const mem_size amount_freed = router_.Free(address);
    tracker_.TrackOperation(address, -pointer_diff(amount_freed) /*negate amount*/);
  }

  auto& tracker() { return tracker_; }

 private:
  MemoryTracker tracker_;
  TRouter router_;
};

// this configures the use of different allocators
// add your own here:
#if (BASE_USE_EQ_ALLOCATOR)
using MemoryCoordinator = MCInstance<EQMemoryRouter>;
#else
using MemoryCoordinator = MCInstance<DefaultCRTRouter>;
#endif

namespace detail {
// no constructor desired
constinit static MemoryCoordinator MemoryRouter{};
}  // namespace


// get the global instance
STRONG_INLINE MemoryCoordinator& memory_coordinator() {
  return detail::MemoryRouter;
}

STRONG_INLINE MemoryTracker& memory_tracker() {
  return detail::MemoryRouter.tracker();
}
}  // namespace base

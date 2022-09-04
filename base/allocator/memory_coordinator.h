// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Guard this entire header against user use.
#define BASE_MAY_USE_MEMORY_COORDINATOR
#ifdef BASE_MAY_USE_MEMORY_COORDINATOR

#include <base/compiler.h>   //< for STRONG_INLINE
#include <base/profiling.h>  //< for BASE_PROFILE_ALLOCATION

#include <base/allocator/allocator_primitives.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#else
#include <base/allocator/default_crt_alloc.h>
#endif

namespace base {

// do not instantiate any complex routers.
// goal is to have these folded in the ::new/alloc operators
template <class TRouter>
struct MCInstance {
  inline void* Allocate(size_t size) {
    void* block = router_.Allocate(size /*void* block*/);
    tracker_.TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return block;
  }

  inline MemoryBlock AllocateAligned(mem_size size, AlignmentValue alignment) {
    void* block = router_.AllocateAligned(
        size /*mem_size size*/,
        static_cast<mem_size>(alignment) /*mem_size alignment*/);

    // TODO: evaluate the alignment properties. when trakcing memory.
    tracker_.TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return {block, size};
  }

  // TODO: tracking.
  inline void* ReAllocate(void* former, mem_size new_size) {
    pointer_diff diff_out = 0; /*already signed*/
    void* block =
        router_.ReAllocate(former /*void* former*/, new_size /*mem_size new_size*/,
                           diff_out /*pointer_diff& diff_out*/);
    tracker_.TrackOperation(block, diff_out);

    BASE_PROFILE_FREE(former);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return block;
  }

  inline MemoryBlock ReAllocateAligned(MemoryBlock former,
                                       mem_size new_size,
                                       AlignmentValue alignment) {
    void* block = router_.ReAllocateAligned(
        former.pointer /*void* former_block*/, former.size /*mem_size former_size*/,
        new_size /*mem_size new_size*/,
        static_cast<mem_size>(alignment) /*mem_size alignment*/);

    // TODO: verify actual new size.
    tracker_.TrackOperation(block, pointer_diff(new_size - former.size));

    BASE_PROFILE_FREE(former);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return {block, new_size};
  }

  inline void Free(void* address) {
    BASE_PROFILE_FREE(address);
    const mem_size amount_freed = router_.Free(address /*void* block*/);
    tracker_.TrackOperation(address, -pointer_diff(amount_freed) /*negate amount*/);
  }

  inline bool Deallocate(MemoryBlock block, AlignmentValue alignment) {
    BASE_PROFILE_FREE(block.pointer);
    bool was_sucessfull = router_.Deallocate(
        block.pointer /*void* block*/, block.size /*mem_size size*/,
        static_cast<mem_size>(alignment) /* mem_size alignment*/);
    tracker_.TrackOperation(block.pointer,
                            -pointer_diff(block.size) /*negate amount*/);
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
}  // namespace detail

// get the global instance
STRONG_INLINE MemoryCoordinator& memory_coordinator() {
  return detail::MemoryRouter;
}

STRONG_INLINE MemoryTracker& memory_tracker() {
  return detail::MemoryRouter.tracker();
}
}  // namespace base

#endif
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Guard this entire header against user use.
#define BASE_MAY_USE_MEMORY_COORDINATOR
#ifdef BASE_MAY_USE_MEMORY_COORDINATOR

#include <base/compiler.h>            //< for STRONG_INLINE
#include <base/profiling/profiler.h>  //< for BASE_PROFILE_ALLOCATION

#include <base/allocator/allocator_primitives.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#else
#include <base/allocator/default_crt_alloc.h>
#endif

namespace base {

BASE_EXPORT MemoryTracker& memory_tracker();

// do not instantiate any complex routers.
// goal is to have these folded in the ::new/alloc operators
template <class TRouter>
struct MCInstance {
  inline void* Allocate(size_t size) {
    void* block = router_.Allocate(size /*void* block*/);
    memory_tracker().TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return block;
  }

  inline allocator_primitives::v2::MemoryBlock AllocateAligned(
      mem_size size,
      allocator_primitives::v2::AlignmentValue alignment) {
    void* block = router_.AllocateAligned(
        size /*mem_size size*/,
        static_cast<mem_size>(alignment) /*mem_size alignment*/);

    // TODO: evaluate the alignment properties. when tracking memory.
    memory_tracker().TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return {block, size};
  }

  // TODO: tracking.
  inline void* ReAllocate(void* former, mem_size new_size) {
    pointer_diff diff_out = 0; /*already signed*/
    void* block = router_.ReAllocate(former /*void* former*/,
                                     new_size /*mem_size new_size*/,
                                     diff_out /*pointer_diff& diff_out*/);
    memory_tracker().TrackOperation(block, diff_out);

    BASE_PROFILE_FREE(former);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return block;
  }

  inline allocator_primitives::v2::MemoryBlock ReAllocateAligned(
      allocator_primitives::v2::MemoryBlock former,
      mem_size new_size,
      allocator_primitives::v2::AlignmentValue alignment) {
    void* block = router_.ReAllocateAligned(
        former.pointer /*void* former_block*/,
        former.size /*mem_size former_size*/, new_size /*mem_size new_size*/,
        static_cast<mem_size>(alignment) /*mem_size alignment*/);

    // TODO: verify actual new size.
    memory_tracker().TrackOperation(block,
                                    pointer_diff(new_size - former.size));

    BASE_PROFILE_FREE(former.pointer);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return {block, new_size};
  }

  inline void Free(void* address) {
    BASE_PROFILE_FREE(address);
    const mem_size amount_freed = router_.Free(address /*void* block*/);
    memory_tracker().TrackOperation(
        address, -pointer_diff(amount_freed) /*negate amount*/);
  }

  inline bool Deallocate(allocator_primitives::v2::MemoryBlock block,
                         allocator_primitives::v2::AlignmentValue alignment) {
    BASE_PROFILE_FREE(block.pointer);
    bool was_sucessfull = router_.Deallocate(
        block.pointer /*void* block*/, block.size /*mem_size size*/,
        static_cast<mem_size>(alignment) /* mem_size alignment*/);
    memory_tracker().TrackOperation(
        block.pointer, -pointer_diff(block.size) /*negate amount*/);
    return was_sucessfull;
  }

 private:
  TRouter router_;
};

// this configures the use of different allocators
// add your own here:
#if (BASE_USE_EQ_ALLOCATOR)
using MemoryCoordinator = MCInstance<EQMemoryRouter>;
#else
using MemoryCoordinator = MCInstance<DefaultCRTRouter>;
#endif

// get the global instance
BASE_EXPORT MemoryCoordinator& memory_coordinator();
}  // namespace base

#endif

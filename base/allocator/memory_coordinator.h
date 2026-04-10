// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Guard this entire header against user use.
#define BASE_MAY_USE_MEMORY_COORDINATOR
#ifdef BASE_MAY_USE_MEMORY_COORDINATOR

#include <base/compiler.h>            //< for STRONG_INLINE
#include <base/export.h>
#include <base/profiling/profiler.h>  //< for BASE_PROFILE_ALLOCATION

#include <base/allocator/allocator_primitives.h>
#include <base/allocator/memory_stat_tracker.h>

#define BASE_USE_EQ_ALLOCATOR 0

#if defined(BASE_USE_MIMALLOC)
#include <base/allocator/mimalloc_router.h>
#elif (BASE_USE_EQ_ALLOCATOR)
#include <base/allocator/eq_alloc/eq_memory_router.h>
#else
#include <base/allocator/default_crt_alloc.h>
#endif

namespace base {

// inline so the compiler can fold TrackOperation into the alloc hot path
// without a function-call indirection.  the static storage has the same
// address across all TUs (C++17 inline variable guarantee).
inline MemoryTracker& memory_tracker() {
  alignas(MemoryTracker) static constinit u8
      storage[sizeof(MemoryTracker)]{};
  return reinterpret_cast<MemoryTracker&>(storage);
}

// do not instantiate any complex routers.
// goal is to have these folded in the ::new/alloc operators
template <class TRouter>
struct MCInstance {
  inline void* Allocate(size_t size) {
    void* block = router_.Allocate(size /*void* block*/);
#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(block, pointer_diff(size));
#endif
    BASE_PROFILE_ALLOCATION(block, size);
    return block;
  }

  inline allocator_primitives::v2::MemoryBlock AllocateAligned(
      mem_size size,
      allocator_primitives::v2::AlignmentValue alignment) {
    void* block = router_.AllocateAligned(
        size /*mem_size size*/, static_cast<mem_size>(alignment) /*mem_size alignment*/);

#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(block, pointer_diff(size));
#endif
    BASE_PROFILE_ALLOCATION(block, size);
    return {block, size};
  }

  inline void* ReAllocate(void* former, mem_size new_size) {
    pointer_diff diff_out = 0; /*already signed*/
    void* block =
        router_.ReAllocate(former /*void* former*/, new_size /*mem_size new_size*/,
                           diff_out /*pointer_diff& diff_out*/);
#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(block, diff_out);
#endif

    BASE_PROFILE_FREE(former);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return block;
  }

  inline allocator_primitives::v2::MemoryBlock ReAllocateAligned(
      allocator_primitives::v2::MemoryBlock former,
      mem_size new_size,
      allocator_primitives::v2::AlignmentValue alignment) {
    void* block = router_.ReAllocateAligned(
        former.pointer /*void* former_block*/, former.size /*mem_size former_size*/,
        new_size /*mem_size new_size*/,
        static_cast<mem_size>(alignment) /*mem_size alignment*/);

#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(block, pointer_diff(new_size - former.size));
#endif

    BASE_PROFILE_FREE(former.pointer);
    BASE_PROFILE_ALLOCATION(block, new_size);
    return {block, new_size};
  }

  inline void Free(void* address) {
    BASE_PROFILE_FREE(address);
    const mem_size amount_freed = router_.Free(address /*void* block*/);
#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(address,
                                    -pointer_diff(amount_freed) /*negate amount*/);
#else
    (void)amount_freed;
#endif
  }

  inline bool Deallocate(allocator_primitives::v2::MemoryBlock block,
                         allocator_primitives::v2::AlignmentValue alignment) {
    BASE_PROFILE_FREE(block.pointer);
    bool was_sucessfull =
        router_.Deallocate(block.pointer /*void* block*/, block.size /*mem_size size*/,
                           static_cast<mem_size>(alignment) /* mem_size alignment*/);
#if defined(X2E_MEMORY_TRACKING)
    memory_tracker().TrackOperation(block.pointer,
                                    -pointer_diff(block.size) /*negate amount*/);
#endif
    return was_sucessfull;
  }

 private:
  TRouter router_;
};

// this configures the use of different allocators
// add your own here:
#if defined(BASE_USE_MIMALLOC)
using MemoryCoordinator = MCInstance<MimallocRouter>;
#elif (BASE_USE_EQ_ALLOCATOR)
using MemoryCoordinator = MCInstance<EQMemoryRouter>;
#else
using MemoryCoordinator = MCInstance<DefaultCRTRouter>;
#endif

// get the global instance
BASE_EXPORT MemoryCoordinator& memory_coordinator();
}  // namespace base

#endif

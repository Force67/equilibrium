// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/threading/spinning_mutex.h>

namespace base {

// Slab-based allocator for small allocations (<= 1 KiB).
//
//   - 8 power-of-2 size classes: 8 .. 1024 bytes
//   - Each 64 KiB page is a slab serving one size class
//   - Thread-local slab ownership — alloc/free operate directly on the
//     slab's intrusive free list (no intermediary cache, no locking)
//   - Bump allocation for fresh slots, free list for recycled slots
//   - Mask-based O(1) slab lookup (ptr & ~0xFFFF → SlabHeader)
class BucketAllocator final : public Allocator {
  friend struct EQMemoryRouter;

 public:
  explicit BucketAllocator(PageTable&);

  void* Allocate(mem_size size, mem_size alignment = 0) override;
  void* ReAllocate(void* former, mem_size new_size, mem_size alignment = 0) override;
  mem_size Free(void* block) override;
  mem_size QueryAllocationSize(void* block) override;

  static constexpr int kNumClasses = 8;
  static constexpr mem_size kClassSizes[kNumClasses] = {8,  16,  32,  64,
                                                        128, 256, 512, 1024};

  static STRONG_INLINE int ClassIndex(mem_size size) {
    if (size <= 8) return 0;
    return static_cast<int>(64 - __builtin_clzll(size - 1)) - 3;
  }

  struct SlabHeader {
    void* free_list;
    SlabHeader* next_page;
    u32 class_index;
    u32 slot_size;
    u32 total_slots;
    u32 bump;
    byte pad_[64 - 32];
  };
  static_assert(sizeof(SlabHeader) == 64, "SlabHeader must be one cache line");

 private:
  // thread-local active slab per class — alloc/free go directly here
  struct ThreadSlabs {
    SlabHeader* active[kNumClasses]{};
  };
  static thread_local ThreadSlabs tl_;

  struct SizeClassBin {
    base::SpinningMutex lock;
  };

  NOINLINE void* AllocateSlow(int idx);
  SlabHeader* NewSlab(int class_index);

  STRONG_INLINE SlabHeader* SlabFromPtr(void* ptr) const {
    return reinterpret_cast<SlabHeader*>(reinterpret_cast<pointer_size>(ptr) &
                                         page_mask_);
  }

  SizeClassBin bins_[kNumClasses]{};
  PageTable& page_table_;
  pointer_size page_mask_{0};
};
}  // namespace base

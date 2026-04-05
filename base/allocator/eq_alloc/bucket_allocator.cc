// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <cstring>
#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/threading/lock_guard.h>

namespace base {

thread_local BucketAllocator::ThreadSlabs BucketAllocator::tl_{};

BucketAllocator::BucketAllocator(PageTable& pt) : page_table_(pt) {
  tl_ = ThreadSlabs{};
  page_mask_ = ~(pt.page_size() - 1);
}

BucketAllocator::SlabHeader* BucketAllocator::NewSlab(int class_index) {
  void* raw = page_table_.RequestPage(PageProtectionFlags::RW, nullptr);
  if (!raw)
    return nullptr;

  // pre-fault system pages to avoid demand-paging during bump alloc
  {
    volatile byte* p = static_cast<volatile byte*>(raw);
    for (mem_size off = 0; off < page_table_.page_size(); off += 4096)
      p[off] = 0;
  }

  const u32 slot_size = static_cast<u32>(kClassSizes[class_index]);
  const u32 usable =
      static_cast<u32>(page_table_.page_size() - sizeof(SlabHeader));

  auto* hdr = reinterpret_cast<SlabHeader*>(raw);
  hdr->free_list = nullptr;
  hdr->next_page = nullptr;
  hdr->class_index = static_cast<u32>(class_index);
  hdr->slot_size = slot_size;
  hdr->total_slots = usable / slot_size;
  hdr->bump = 0;
  return hdr;
}

// slow: get a fresh slab
void* BucketAllocator::AllocateSlow(int idx) {
  SlabHeader* slab;
  {
    base::NonOwningScopedLockGuard _(bins_[idx].lock);
    (void)_;
    slab = NewSlab(idx);
  }
  if (!slab)
    return nullptr;
  tl_.active[idx] = slab;

  // bump first slot
  byte* base = reinterpret_cast<byte*>(slab) + sizeof(SlabHeader);
  void* slot = base;
  slab->bump = 1;
  return slot;
}

void* BucketAllocator::Allocate(mem_size size, mem_size /*alignment*/) {
  const int idx = ClassIndex(size);
  SlabHeader* slab = tl_.active[idx];

  if (slab) {
    // 1) reuse a freed slot (hot path after frees)
    if (slab->free_list) {
      void* slot = slab->free_list;
      slab->free_list = *reinterpret_cast<void**>(slot);
      return slot;
    }
    // 2) bump a fresh slot (hot path for sequential allocs)
    if (slab->bump < slab->total_slots) {
      byte* base = reinterpret_cast<byte*>(slab) + sizeof(SlabHeader);
      void* slot = base + static_cast<mem_size>(slab->bump) * slab->slot_size;
      slab->bump++;
      return slot;
    }
  }

  // 3) slab exhausted or missing — get a new one
  return AllocateSlow(idx);
}

mem_size BucketAllocator::Free(void* block) {
  if (!block)
    return 0;

  // O(1) slab lookup via AND mask, then push to slab's intrusive free list.
  // same structure as mimalloc: 1 AND + 2 stores.
  SlabHeader* slab = SlabFromPtr(block);
  *reinterpret_cast<void**>(block) = slab->free_list;
  slab->free_list = block;
  return slab->slot_size;
}

void* BucketAllocator::ReAllocate(void* former_block,
                                  mem_size new_size,
                                  mem_size user_alignment) {
  if (!former_block)
    return Allocate(new_size, user_alignment);
  if (new_size == 0) {
    Free(former_block);
    return nullptr;
  }
  SlabHeader* slab = SlabFromPtr(former_block);
  if (new_size <= slab->slot_size)
    return former_block;
  void* new_block = Allocate(new_size, user_alignment);
  if (!new_block)
    return nullptr;
  std::memcpy(new_block, former_block, slab->slot_size);
  Free(former_block);
  return new_block;
}

mem_size BucketAllocator::QueryAllocationSize(void* block) {
  if (!block)
    return 0;
  SlabHeader* slab = SlabFromPtr(block);
  return slab ? slab->slot_size : 0;
}
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/threading/spinning_mutex.h>

namespace base {

// HeapAllocator services allocations larger than kPageThreshold (64 KiB).
// Design inspired by Bluepoint's memory system (GDC 2019):
//   - Each allocation is backed by one or more contiguous 64 KiB pages
//   - A BlockHeader is placed at the start of every allocation
//   - Freed blocks are kept in a doubly-linked free list sorted by size
//     (ascending) so the first match is best-fit
//   - Reuse from the free list avoids expensive virtual memory syscalls
//   - Thread safety via SpinningMutex (atomic lock, no mutex)
class HeapAllocator final : public Allocator {
 public:
  explicit HeapAllocator(PageTable& page_table);

  void* Allocate(mem_size size, mem_size user_alignment = 1024) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 1024) override;

  mem_size Free(void* block) override;

  mem_size QueryAllocationSize(void* block) override;

 private:
  // placed at the start of every heap allocation, immediately before user data.
  // free blocks are chained via next_free/prev_free into a size-sorted list
  // for best-fit lookup.
  struct BlockHeader {
    static constexpr u32 kMagic = 0xEA110C8D;

    u32 magic;
    u32 page_count;       // number of backing pages
    mem_size total_size;  // page_count * page_size (committed bytes)
    mem_size user_size;   // what the caller requested

    enum Flags : u32 { kFree = 0, kInUse = 1 };
    u32 flags;
    u32 reserved_;

    BlockHeader* next_free;
    BlockHeader* prev_free;

    byte* UserData() { return reinterpret_cast<byte*>(this + 1); }

    static BlockHeader* FromUserData(void* ptr) {
      return reinterpret_cast<BlockHeader*>(static_cast<byte*>(ptr) -
                                            sizeof(BlockHeader));
    }

    bool Validate() const { return magic == kMagic; }
  };
  static_assert(sizeof(BlockHeader) == 48, "BlockHeader size unexpected");

  mem_size PagesRequired(mem_size user_size) const;

  // free list management (caller must hold lock_)
  void FreeListInsert(BlockHeader* block);
  void FreeListRemove(BlockHeader* block);
  BlockHeader* FreeListFindBestFit(mem_size required_pages);

  PageTable& page_table_;
  base::SpinningMutex lock_;
  BlockHeader* free_list_{nullptr};
};
}  // namespace base

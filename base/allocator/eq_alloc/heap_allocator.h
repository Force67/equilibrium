// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/threading/spinning_mutex.h>

namespace base {

// HeapAllocator services allocations larger than kPageThreshold (64 KiB).
//   - Pre-committed arena for fast bump allocation (no mmap per alloc)
//   - Freed blocks kept in a size-sorted free list for best-fit reuse
//   - Thread safety via SpinningMutex
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
  struct BlockHeader {
    static constexpr u32 kMagic = 0xEA110C8D;

    u32 magic;
    u32 page_count;
    mem_size total_size;
    mem_size user_size;

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

  void FreeListInsert(BlockHeader* block);
  void FreeListRemove(BlockHeader* block);
  BlockHeader* FreeListFindBestFit(mem_size required_pages);

  // grow the arena by committing a new chunk
  bool GrowArena();

  PageTable& page_table_;
  base::SpinningMutex lock_;
  BlockHeader* free_list_{nullptr};

  // pre-committed arena: bump-allocate from here to avoid per-alloc mmap
  static constexpr mem_size kArenaChunkPages = 256;  // 16 MiB per chunk
  byte* arena_base_{nullptr};
  byte* arena_cursor_{nullptr};
  byte* arena_end_{nullptr};
  mem_size page_size_{0};
};
}  // namespace base

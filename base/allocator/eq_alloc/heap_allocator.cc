// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/heap_allocator.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/threading/lock_guard.h>

#include <cstring>

namespace base {

HeapAllocator::HeapAllocator(PageTable& page_table) : page_table_(page_table) {}

mem_size HeapAllocator::PagesRequired(mem_size user_size) const {
  const mem_size ps = page_table_.page_size();
  return (user_size + sizeof(BlockHeader) + ps - 1) / ps;
}

// ---- free list (size-sorted, ascending) ----

void HeapAllocator::FreeListInsert(BlockHeader* block) {
  block->flags = BlockHeader::kFree;
  block->next_free = nullptr;
  block->prev_free = nullptr;

  if (!free_list_) {
    free_list_ = block;
    return;
  }

  // walk until we find a node whose total_size >= block's
  BlockHeader* prev = nullptr;
  BlockHeader* curr = free_list_;
  while (curr && curr->total_size < block->total_size) {
    prev = curr;
    curr = curr->next_free;
  }

  block->next_free = curr;
  block->prev_free = prev;
  if (prev)
    prev->next_free = block;
  else
    free_list_ = block;
  if (curr)
    curr->prev_free = block;
}

void HeapAllocator::FreeListRemove(BlockHeader* block) {
  if (block->prev_free)
    block->prev_free->next_free = block->next_free;
  else
    free_list_ = block->next_free;
  if (block->next_free)
    block->next_free->prev_free = block->prev_free;
  block->next_free = nullptr;
  block->prev_free = nullptr;
}

HeapAllocator::BlockHeader* HeapAllocator::FreeListFindBestFit(
    mem_size required_pages) {
  const mem_size required_size = required_pages * page_table_.page_size();
  // list is sorted ascending by total_size, so the first block that fits is
  // best-fit
  BlockHeader* curr = free_list_;
  while (curr) {
    if (curr->total_size >= required_size)
      return curr;
    curr = curr->next_free;
  }
  return nullptr;
}

// ---- Allocator interface ----

void* HeapAllocator::Allocate(mem_size size, mem_size /*user_alignment*/) {
  if (size == 0)
    return nullptr;

  const mem_size pages = PagesRequired(size);

  base::NonOwningScopedLockGuard _(lock_);
  (void)_;

  // try to reuse a freed block
  if (BlockHeader* block = FreeListFindBestFit(pages)) {
    FreeListRemove(block);
    block->flags = BlockHeader::kInUse;
    block->user_size = size;
    return block->UserData();
  }

  // allocate fresh pages
  mem_size allocated_size = 0;
  void* memory =
      page_table_.RequestPages(pages, PageProtectionFlags::RW, &allocated_size);
  if (!memory) {
    DEBUG_TRAP;
    return nullptr;
  }

  // place header at page base
  auto* header = reinterpret_cast<BlockHeader*>(memory);
  header->magic = BlockHeader::kMagic;
  header->page_count = static_cast<u32>(pages);
  header->total_size = allocated_size;
  header->user_size = size;
  header->flags = BlockHeader::kInUse;
  header->next_free = nullptr;
  header->prev_free = nullptr;

  return header->UserData();
}

void* HeapAllocator::ReAllocate(void* former_block,
                                mem_size new_size,
                                mem_size user_alignment) {
  if (!former_block)
    return Allocate(new_size, user_alignment);
  if (new_size == 0) {
    Free(former_block);
    return nullptr;
  }

  auto* header = BlockHeader::FromUserData(former_block);
  BASE_DCHECK(header->Validate(), "HeapAllocator::ReAllocate: corrupt header");

  const mem_size old_user_size = header->user_size;

  // if the existing span already has enough room, just update user_size
  if (PagesRequired(new_size) <= header->page_count) {
    header->user_size = new_size;
    return former_block;
  }

  // growing: allocate new, copy, free old
  void* new_block = Allocate(new_size, user_alignment);
  if (!new_block)
    return nullptr;

  const mem_size copy_size = old_user_size < new_size ? old_user_size : new_size;
  std::memcpy(new_block, former_block, copy_size);
  Free(former_block);
  return new_block;
}

mem_size HeapAllocator::Free(void* block) {
  if (!block)
    return 0;

  auto* header = BlockHeader::FromUserData(block);
  BASE_DCHECK(header->Validate(), "HeapAllocator::Free: corrupt header");
  BASE_DCHECK(header->flags == BlockHeader::kInUse,
         "HeapAllocator::Free: double free");

  const mem_size freed = header->user_size;

  base::NonOwningScopedLockGuard _(lock_);
  (void)_;
  FreeListInsert(header);
  return freed;
}

mem_size HeapAllocator::QueryAllocationSize(void* block) {
  if (!block)
    return 0;
  auto* header = BlockHeader::FromUserData(block);
  BASE_DCHECK(header->Validate(),
         "HeapAllocator::QueryAllocationSize: corrupt header");
  return header->user_size;
}
}  // namespace base

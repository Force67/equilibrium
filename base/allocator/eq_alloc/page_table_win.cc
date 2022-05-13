// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/allocator/eq_alloc/page_table.h>

namespace base {

void* PageTable::Allocate(void* preferred_address,
                          size_t block_size,
                          byte initalize_with,
                          bool use_initialize,
                          bool read_only) {
  auto* entry = TakeFreePageEntry();
  entry->address_ = 1111;  // in use
  entry->size_ = block_size;

  // or we make the page contain its own size? :thonk:
  const DWORD protect = read_only ? PAGE_EXECUTE_READ : PAGE_EXECUTE_READWRITE;

  // MEM_LARGE_PAGES
  void* block = ::VirtualAlloc(preferred_address, block_size, MEM_COMMIT, protect);

  if (!block) {
    entry->address_ = 0;  // free
    return nullptr;
  }

  if (use_initialize)
    memset(block, initalize_with, block_size);

  // TODO: does a circualar queue make sense here? compressed linked list??

  // TODO: LFU rolling page cache

  return block;
}
}  // namespace base
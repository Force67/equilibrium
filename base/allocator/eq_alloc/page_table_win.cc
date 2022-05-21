// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/allocator/eq_alloc/page_table.h>

namespace base {
namespace {
using namespace memory_literals;
}  // namespace

u32 PageTable::ideal_page_size() {
  return static_cast<u32>(64_kib);
}

byte* PageTable::Reserve(void* preferred_address, size_t block_size) {
  return reinterpret_cast<byte*>(::VirtualAlloc(
      preferred_address, block_size, MEM_RESERVE, PAGE_EXECUTE_READWRITE));
}

void* PageTable::Allocate(void* preferred_address,
                          size_t block_size,
                          byte initalize_with,
                          bool use_initialize,
                          bool read_only) {
  // TODO: think about MEM_LARGE_PAGES
  // TODO: does a circualar queue make sense here? compressed linked list??
  // TODO: LFU rolling page cache

  const DWORD protect = read_only ? PAGE_EXECUTE_READ : PAGE_EXECUTE_READWRITE;
  void* block = ::VirtualAlloc(preferred_address, block_size, MEM_COMMIT, protect);

  if (!block)
    return nullptr;

  if (use_initialize)
    memset(block, initalize_with, block_size);

  return block;
}
}  // namespace base
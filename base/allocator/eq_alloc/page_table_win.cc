// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/allocator/eq_alloc/page_table.h>

namespace base {
namespace {
using namespace memory_literals;
}  // namespace

u32 PageTable::current_page_size() {
  SYSTEM_INFO sys_info{};
  ::GetSystemInfo(&sys_info);
  return sys_info.dwPageSize;
}

u32 PageTable::ideal_page_size() {
  return static_cast<u32>(64_kib);
}

byte* PageTable::Reserve(void* preferred_address, mem_size block_size) {
  return reinterpret_cast<byte*>(::VirtualAlloc(
      // touching a unallocated page should result in immedeate infraction.
      preferred_address, block_size, MEM_RESERVE, PAGE_NOACCESS));
}

void* PageTable::Allocate(void* preferred_address,
                          mem_size block_size,
                          base::PageProtectionFlags page_protection_flags,
                          byte initalize_with,
                          bool use_initialize) {
  // TODO: think about MEM_LARGE_PAGES
  // TODO: does a circualar queue make sense here? compressed linked list??
  // TODO: LFU rolling page cache
  const DWORD protect_flags =
      base::TranslateToNativePageProtection(page_protection_flags);

  void* block =
      ::VirtualAlloc(preferred_address, block_size, MEM_COMMIT, protect_flags);

  if (!block)
    return nullptr;

  if (use_initialize)
    memset(block, initalize_with, block_size);

  return block;
}
}  // namespace base
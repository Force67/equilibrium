// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

#pragma comment(lib, "mincore")

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
  return static_cast<u32>(eq_allocation_constants::kPageThreshold);
}

u32 PageTable::page_boundary_alignment() {
  return static_cast<u32>(1_mib);
}

byte* PageTable::Reserve(void* requested_range_start_address,
                         void* requested_range_end_address,
                         mem_size block_size) {
  return reinterpret_cast<byte*>(::VirtualAlloc(
      // touching a unallocated page should result in immedeate infraction.
      requested_range_start_address, block_size, MEM_RESERVE, PAGE_NOACCESS));
}

void* PageTable::Allocate(void* requested_range_start_address,
                          mem_size block_size,
                          base::PageProtectionFlags page_protection_flags,
                          byte initalize_with,
                          bool use_initialize) {
  // TODO: think about MEM_LARGE_PAGES
  // TODO: does a circualar queue make sense here? compressed linked list??
  // TODO: LFU rolling page cache
  const DWORD protect_flags =
      base::TranslateToNativePageProtection(page_protection_flags);
  PVOID address =
      ::VirtualAlloc2(::GetCurrentProcess(), requested_range_start_address,
                      block_size, MEM_COMMIT, protect_flags, nullptr, 0);
  if (!address)
    return nullptr;

  if (use_initialize)
    memset(address, initalize_with, block_size);

  return address;
}

bool PageTable::DeAllocate(void* block_address) {
  if (!block_address)
    return false;

  // ::VirtualFree will return non-zero value on success, which we can then use to
  // return true/false
  return ::VirtualFree(block_address, 0, MEM_RELEASE) != 0;
}

bool PageTable::ChangePageProtection(
    void* block_address,
    mem_size block_size,
    base::PageProtectionFlags new_protection_flags) {
  const DWORD new_protect_flags =
      base::TranslateToNativePageProtection(new_protection_flags);
  DWORD old_protect_flags;

  return ::VirtualProtect(block_address, block_size, new_protect_flags,
                          &old_protect_flags) != 0;
}
}  // namespace base
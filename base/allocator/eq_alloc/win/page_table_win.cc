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
  // from non fixed base
  if (!requested_range_start_address) {
    return reinterpret_cast<byte*>(::VirtualAlloc(
        // touching a unallocated page should result in immedeate infraction.
        nullptr, block_size, MEM_RESERVE, PAGE_NOACCESS));
  }

  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  if (reinterpret_cast<uintptr_t>(requested_range_end_address) >
      reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress)) {
    __debugbreak();
    return nullptr;
  }

  uintptr_t start_address =
      reinterpret_cast<uintptr_t>(requested_range_start_address);
  // Ensure requested_range_start_address is a multiple of the allocation granularity
  if (start_address % sysInfo.dwAllocationGranularity != 0) {
    start_address += sysInfo.dwAllocationGranularity -
                     (start_address % sysInfo.dwAllocationGranularity);
    requested_range_start_address = reinterpret_cast<void*>(start_address);
  }

  // Ensure block_size is a multiple of the page size
  if (block_size % sysInfo.dwPageSize != 0) {
    __debugbreak();
    return nullptr;
  }

  SIZE_T largePageSize = GetLargePageMinimum();
  if (block_size % largePageSize != 0) {
    __debugbreak();
    return nullptr;
  }

  MEM_ADDRESS_REQUIREMENTS memReq = {
      .LowestStartingAddress =
          requested_range_start_address, /*Specifies the lowest acceptable address.
                                            This address must be a multiple of the
                                            allocation granularity returned by
                                            GetSystemInfo, or a multiple of the large
                                            page size returned by GetLargePageMinimum
                                            if large pages are being requested. If
                                            this member is NULL, then there is no
                                            lower limit.*/
      .HighestEndingAddress =
          requested_range_end_address, /*Specifies the highest acceptable address
                                          (inclusive). This address must not exceed
                                          lpMaximumApplicationAddress returned by
                                          GetSystemInfo. If this member is NULL, then
                                          there is no upper limit.*/
      .Alignment = 0 /*Specifies power-of-2 alignment. Specifying 0 aligns the
                        returned address on the system allocation granularity.

*/};
  MEM_EXTENDED_PARAMETER param = {.Type = MemExtendedParameterAddressRequirements,
                                  .Pointer = &memReq};
  DWORD err222 = ::GetLastError();

  auto* ptr = reinterpret_cast<byte*>(::VirtualAlloc2(
      nullptr,
      // touching a unallocated page should result in immedeate infraction.
      nullptr, block_size, MEM_RESERVE, PAGE_READWRITE, &param,
      1));

  DWORD err = ::GetLastError();
  return ptr;
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
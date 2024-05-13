// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/enum_traits.h>

namespace base {
enum class PageProtectionFlags : u32 {
  NONE = 0x0,  // No Access

  R = 0x1,  // Read
  W = 0x2,  // Write
  X = 0x4,  // Execute
  C = 0x8,  // Copy On Write

  G = 0x10,  // Guard

  INVALID = 0x80000000,  // Invalid

  RW = R | W,
  RX = R | X,
  RWX = R | W | X,
  RWC = R | W | C,
  RWXC = R | W | X | C,
};
BASE_IMPL_ENUM_BIT_TRAITS(PageProtectionFlags, u32)

#if defined(OS_WIN)
using NativePageProtectionType = u32;
#elif defined(OS_POSIX)
using NativePageProtectionType = int;  // whatever they define as int.
#endif

NativePageProtectionType TranslateToNativePageProtection(
    const PageProtectionFlags);
PageProtectionFlags TranslateFromNativePageProtection(
    const NativePageProtectionType);

u32 FetchCurrentPageSize();

// Reserves a range of the process's virtual address space without allocating
// any actual physical storage in memory or in the paging file on disk.
// use this for blocking out a range of memory that you want to allocate later.
byte* VirtualMemoryReserve(void* address, mem_size size);

// Allocates memory charges (from the overall size of memory and the paging
// files on disk) for the specified reserved memory pages.
// If you already reserved memory with VirtualMemoryReserve, this will work fine
// too.
byte* VirtualMemoryAllocate(void* address,
                            mem_size size,
                            PageProtectionFlags protection, const bool reserve = true);

// Releases memory that was previously reserved with VirtualMemoryReserve
bool VirtualMemoryFree(void* address, mem_size size);

bool VirtualMemoryProtect(void* address,
                          mem_size size,
                          PageProtectionFlags protection);
}  // namespace base
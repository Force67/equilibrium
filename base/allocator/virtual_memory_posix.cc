// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <sys/mman.h>
#include <cstdint>   // for uint32_t
#include <cstring>   // for memset
#include <unistd.h>  // for sysconf
#include "virtual_memory.h"

namespace base {

// Translate base::PageProtectionFlags to Linux-specific protection flags.
NativePageProtectionType TranslateToNativePageProtection(
    const PageProtectionFlags flags) {
  NativePageProtectionType result = PROT_NONE;

  if (flags & PageProtectionFlags::X) {
    if (flags & PageProtectionFlags::C) {
      result = PROT_EXEC | PROT_WRITE;
    } else if (flags & PageProtectionFlags::W) {
      result = PROT_EXEC | PROT_READ | PROT_WRITE;
    } else if (flags & PageProtectionFlags::R) {
      result = PROT_EXEC | PROT_READ;
    } else {
      result = PROT_EXEC;
    }
  } else {
    if (flags & PageProtectionFlags::C) {
      result = PROT_WRITE;
    } else if (flags & PageProtectionFlags::W) {
      result = PROT_READ | PROT_WRITE;
    } else if (flags & PageProtectionFlags::R) {
      result = PROT_READ;
    } else {
      result = PROT_NONE;
    }
  }

  // DCHECK(!(flags & PageProtectionFlags::G), "Posix doesnt support guard pages
  // in the same way");
  return result;
}

// Translate Linux-specific protection flags to base::PageProtectionFlags.
PageProtectionFlags TranslateFromNativePageProtection(
    const NativePageProtectionType flags) {
  PageProtectionFlags result = PageProtectionFlags::NONE;

  switch (flags & (PROT_READ | PROT_WRITE | PROT_EXEC)) {
    case PROT_EXEC:
      result = PageProtectionFlags::X;
      break;
    case PROT_EXEC | PROT_READ:
      result = PageProtectionFlags::RX;
      break;
    case PROT_EXEC | PROT_READ | PROT_WRITE:
      result = PageProtectionFlags::RWX;
      break;
    case PROT_EXEC | PROT_WRITE:
      result = PageProtectionFlags::RWXC;
      break;
    case PROT_READ:
      result = PageProtectionFlags::R;
      break;
    case PROT_READ | PROT_WRITE:
      result = PageProtectionFlags::RW;
      break;
    case PROT_WRITE:
      result = PageProtectionFlags::RWC;
      break;
    default:
      result = PageProtectionFlags::INVALID;
      break;
  }

  // if (flags & MAP_GUARD)
  //     result |= PageProtectionFlags::G;

  return result;
}

uint32_t FetchCurrentPageSize() {
  long page_size = ::sysconf(_SC_PAGESIZE);
  if (page_size == -1) {
    return 4096;  // Default page size on many systems
  }
  return static_cast<uint32_t>(page_size);
}

byte* VirtualMemoryReserve(void* address, mem_size size) {
  int flags = MAP_ANONYMOUS | MAP_PRIVATE;
  if (address) {
    flags |= MAP_FIXED;
  }
  return reinterpret_cast<byte*>(::mmap(address, size, PROT_NONE, flags, -1, 0));
}

byte* VirtualMemoryAllocate(void* address,
                            mem_size size,
                            PageProtectionFlags protection,
                            const bool reserve) {
  int prot = static_cast<int>(protection);
  int flags = MAP_ANONYMOUS | MAP_PRIVATE;
  if (address) {
    flags |= MAP_FIXED;
  }
  if (reserve) {
    flags |= MAP_NORESERVE;  // Don't commit physical memory yet
  }
  return reinterpret_cast<byte*>(::mmap(address, size, prot, flags, -1, 0));
}

bool VirtualMemoryFree(void* address, mem_size size) {
  return ::munmap(address, size) == 0;
}

bool VirtualMemoryProtect(void* address, mem_size size, PageProtectionFlags protection) {
  int prot = static_cast<int>(protection);
  return ::mprotect(address, size, prot) == 0;
}
}  // namespace base

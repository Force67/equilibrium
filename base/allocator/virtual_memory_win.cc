// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/allocator/virtual_memory.h>

namespace base {
// https://github.com/0x1F9F1/mem/blob/master/include/mem/PageProtectionFlags.h
NativePageProtectionType TranslateToNativePageProtection(
    const PageProtectionFlags flags) {
  NativePageProtectionType result = PAGE_NOACCESS;

  if (flags & PageProtectionFlags::X) {
    if (flags & PageProtectionFlags::C) {
      result = PAGE_EXECUTE_WRITECOPY;
    } else if (flags & PageProtectionFlags::W) {
      result = PAGE_EXECUTE_READWRITE;
    } else if (flags & PageProtectionFlags::R) {
      result = PAGE_EXECUTE_READ;
    } else {
      result = PAGE_EXECUTE;
    }
  } else {
    if (flags & PageProtectionFlags::C) {
      result = PAGE_EXECUTE_WRITECOPY;
    } else if (flags & PageProtectionFlags::W) {
      result = PAGE_READWRITE;
    } else if (flags & PageProtectionFlags::R) {
      result = PAGE_READONLY;
    } else {
      result = PAGE_NOACCESS;
    }
  }

  if (flags & PageProtectionFlags::G)
    result |= PAGE_GUARD;

  return result;
}

PageProtectionFlags TranslateFromNativePageProtection(
    const NativePageProtectionType flags) {
  PageProtectionFlags result = PageProtectionFlags::NONE;

  switch (flags & 0xFF) {
    case PAGE_EXECUTE:
      result = PageProtectionFlags::X;
      break;
    case PAGE_EXECUTE_READ:
      result = PageProtectionFlags::RX;
      break;
    case PAGE_EXECUTE_READWRITE:
      result = PageProtectionFlags::RWX;
      break;
    case PAGE_EXECUTE_WRITECOPY:
      result = PageProtectionFlags::RWXC;
      break;
    case PAGE_NOACCESS:
      result = PageProtectionFlags::NONE;
      break;
    case PAGE_READONLY:
      result = PageProtectionFlags::R;
      break;
    case PAGE_READWRITE:
      result = PageProtectionFlags::RW;
      break;
    case PAGE_WRITECOPY:
      result = PageProtectionFlags::RWC;
      break;
    default:
      result = PageProtectionFlags::INVALID;
      break;
  }

  if (flags & PAGE_GUARD)
    result |= PageProtectionFlags::G;

  return result;
}

u32 FetchCurrentPageSize() {
  SYSTEM_INFO sys_info{};
  ::GetSystemInfo(&sys_info);
  return sys_info.dwPageSize;
}

byte* VirtualMemoryReserve(void* address, mem_size size) {
  const DWORD native_protection = PAGE_NOACCESS;
  return reinterpret_cast<byte*>(::VirtualAlloc(
      address, size, MEM_RESERVE, native_protection));  // MEM_RELEASE for free
}

byte* base::VirtualMemoryAllocate(void* address,
                                  mem_size size,
                                  PageProtectionFlags protection,
                                  const bool reserve) {
  const DWORD native_protection = TranslateToNativePageProtection(protection);
  DWORD allocation_type = MEM_COMMIT;
  if (reserve) {
    allocation_type |= MEM_RESERVE;
  }
  return reinterpret_cast<byte*>(
      ::VirtualAlloc(address, size, allocation_type, native_protection));
}

bool VirtualMemoryFree(void* address, mem_size size) {
  return ::VirtualFree(address, 0, MEM_RELEASE) != 0;
}

bool VirtualMemoryProtect(void* address,
                          mem_size size,
                          PageProtectionFlags protection) {
  DWORD old_protection;
  return ::VirtualProtect(address, size,
                          TranslateToNativePageProtection(protection),
                          &old_protection) != 0;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <sys/mman.h>
#include <unistd.h>
#include <cstring> // for memset

#include <base/allocator/eq_alloc/page_table.h>
#include "arch.h"
#include "check.h"

namespace base {
namespace {
using namespace memory_literals;
}  // namespace

u32 PageTable::ideal_page_size() {
  return static_cast<u32>(64_kib);
}

u32 PageTable::current_page_size() {
  return static_cast<u32>(sysconf(_SC_PAGESIZE));
}

u32 PageTable::page_boundary_alignment() {
  return static_cast<u32>(1_mib);
}

byte* PageTable::Reserve(void* requested_range_start_address,
                         void* requested_range_end_address,
                         mem_size block_size) {
    // On POSIX, memory reservation and protection are generally handled together
    // using mmap with MAP_ANONYMOUS | MAP_PRIVATE flags.
    return reinterpret_cast<byte*>(mmap(requested_range_start_address,
                                        block_size, PROT_NONE,
                                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}

void* PageTable::Allocate(void* requested_range_start_address,
                          mem_size block_size,
                          base::PageProtectionFlags page_protection_flags,
                          byte initialize_with,
                          bool use_initialize) {
    NativePageProtectionType protect_flags = base::TranslateToNativePageProtection(page_protection_flags);;
    // MAP_ANONYMOUS is used to allocate memory without using a file.
    // MAP_PRIVATE creates a private copy-on-write mapping.
    void* address = mmap(requested_range_start_address, block_size, protect_flags,
                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (address == MAP_FAILED)
        return nullptr;

    if (use_initialize)
        memset(address, initialize_with, block_size);

    return address;
}

bool PageTable::DeAllocate(void* block_address) {
    if (!block_address)
        return false;

    // munmap returns 0 on success, so we can use !0 to check for success.
    return munmap(block_address, 0) == 0;
}

bool PageTable::ChangePageProtection(
    void* block_address,
    mem_size block_size,
    base::PageProtectionFlags new_protection_flags) {
    int new_protect_flags = base::TranslateToNativePageProtection(new_protection_flags);

    // mprotect returns 0 on success, so we can use !0 to check for success.
    return mprotect(block_address, block_size, new_protect_flags) == 0;
}
}  // namespace base
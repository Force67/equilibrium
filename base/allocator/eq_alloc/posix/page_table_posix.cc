// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

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
  return 0;
}

u32 PageTable::page_boundary_alignment() {
  return static_cast<u32>(1_mib);
}

byte* PageTable::Reserve(void* preferred_address, mem_size block_size) {
  return nullptr;
}

void* PageTable::Allocate(void* preferred_address,
                          mem_size block_size,
                          base::PageProtectionFlags page_protection_flags,
                          byte initalize_with,
                          bool use_initialize) {
  // TODO: think about MEM_LARGE_PAGES
  // TODO: does a circualar queue make sense here? compressed linked list??
  // TODO: LFU rolling page cache

  IMPOSSIBLE;
  return nullptr;
}
}  // namespace base
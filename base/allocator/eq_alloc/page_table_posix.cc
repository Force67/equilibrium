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

byte* PageTable::Reserve(void* preferred_address, mem_size block_size) {
  return nullptr;
}

void* PageTable::Allocate(void* preferred_address,
                          mem_size block_size,
                          byte initalize_with,
                          bool use_initialize,
                          bool read_only) {
  // TODO: think about MEM_LARGE_PAGES
  // TODO: does a circualar queue make sense here? compressed linked list??
  // TODO: LFU rolling page cache

    IMPOSSIBLE;
  return nullptr;
}
}  // namespace base
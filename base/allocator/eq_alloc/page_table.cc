// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include "arch.h"
#include "compiler.h"

namespace base {

namespace {
constexpr mem_size kPageGrowByRatio = 2;
}

PageTable::PageTable() {
  ReservePages(0, kPageGrowByRatio);
}

// Take n pages
mem_size PageTable::ReservePages(const pointer_size page_base, const mem_size page_count) {
  const mem_size total_page_memory = ideal_page_size() * page_count;
  byte* preferred_address = page_base ? reinterpret_cast<byte*>(page_base) : nullptr;

  byte* block = Reserve(preferred_address, ideal_page_size() * total_page_memory);
  // no pages were allocated
  if (!block)
    return 0;

  // first block sets this
  if (!first_page_)
    first_page_ = reinterpret_cast<pointer_size>(block);

  for (mem_size i = 0; i < page_count; i++) {
    auto& e = page_entries_[i];
    e.address = reinterpret_cast<pointer_size>(block);

    block += ideal_page_size();
    // no point in setting the size, it indicates the free'ness
  }

  return page_count;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags, mem_size* size_out) {
  // enter lock...
  for (auto i = 0; i < 12; i++) {
    auto& e = page_entries_[i];

    // in use.
    if (e.size != 0)
      continue;

    // for debugging reasons we fill each new page with 0xFF to detect unitialized
    // memory more easily
    if (void* block =
            Allocate(e.pointer(), ideal_page_size(), page_flags, 0xFF, true)) {
      // this marks the block as 'in-use', so we only do it on success
      e.size = ideal_page_size();

      if (size_out)
        *size_out = e.size;

      return block;
    }
  }

  return nullptr;
}

PageTable::PageEntry* PageTable::FindBackingPage(void* block) {
  for (auto i = 0; i < 12; i++) {
    auto& e = page_entries_[i];
    if (e.Contains(block)) {
      return &e;
    }
  }
  return nullptr;
}

bool PageTable::Free(void* block) {
  // TODO: OS Free it
  PageEntry* entry = FindBackingPage(block);
  if (!entry)
    return false;

  entry->size = 0;
  return true;
}
}  // namespace base
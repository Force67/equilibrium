// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_table.h>

namespace base {

PageTable::PageEntry* PageTable::TakeFreePageEntry() {
  // these need locks..
  size_t i = 0;
  // still will cannibalize start?
  while (i <= kPageLimit && entries_[i].address_ != 0)
    i++;
  return &entries_[i];
}

PageTable::PageEntry* PageTable::FindBackingPage(void* block) {
#if 0
  size_t i = 0;
  while (i <= kPageLimit && entries_[i].address_ != 0) {
    if (entries_[i].Contains(block)) {
      return &entries_[i];
    }
    i++;
  }
#endif
  return nullptr;
}

bool PageTable::Free(void* block) {
#if 0
  size_t i = 0;
  while (i < kPageLimit && entries_[i].address_ != 0) {
    entries_[i].address_ == block;
  }
#endif

  return false;
}
}  // namespace base
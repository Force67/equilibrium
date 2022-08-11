// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include "arch.h"
#include "compiler.h"

namespace base {

namespace {
constexpr mem_size kPageGrowByRatio = 2;

class spin_lock {
  static constexpr int UNLOCKED = 0;
  static constexpr int LOCKED = 1;

  base::Atomic<int> m_value = 0;

 public:
  void lock() {
    while (true) {
      int expected = UNLOCKED;
      if (m_value.compare_exchange_weak(expected, LOCKED))
        break;
    }
  }

  void unlock() { m_value.store(UNLOCKED); }
};

class ScopedSpinLock : spin_lock {
 public:
  ScopedSpinLock() { spin_lock::lock(); }

  ~ScopedSpinLock() { spin_lock::unlock(); }
};
}  // namespace

PageTable::PageTable() {
  ReservePages(0, kPageGrowByRatio);
}

mem_size PageTable::ReservePages(const pointer_size page_base,
                                 const mem_size page_reserve_count) {
  byte* preferred_address = page_base ? reinterpret_cast<byte*>(page_base) : nullptr;

  const mem_size page_boundary = page_boundary_alignment();
  const mem_size page_size = ideal_page_size();

  byte* block = nullptr;
  if (page_boundary == 0) {
    // no boundary, we simply can tack on the next page on the end of the current
    // page so we can allocate everything in one go
    const mem_size total_page_memory = page_size * page_reserve_count;
    block = Reserve(preferred_address, page_size * total_page_memory);
  } else {
    block = preferred_address;

    for (auto i = 0; i < page_reserve_count; i++) {
      block = Reserve(block, page_size);
      if (!block)
        return 0;

      // first block sets this
      if (!first_page_)
        first_page_ = reinterpret_cast<pointer_size>(block);

      // TODO, this isnt exactly aligning at page bounds..
      block += page_boundary;
    }
  }

  // no pages were allocated
  if (!block)
    return 0;

  return page_reserve_count;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags, mem_size* size_out) {
  //ScopedSpinLock _;

    #if 0
  if (void* block =
          Allocate(e.pointer(), ideal_page_size(), page_flags, 0xFF, true)) {
    return block;
  }
  #endif

  // walk the freelist..

      // for debugging reasons we fill each new page with 0xFF to detect unitialized
  // memory more easily
  #if 0
  if (void* block =
          Allocate(e.pointer(), ideal_page_size(), page_flags, 0xFF, true)) {
    return block;
  }
  #endif

  return nullptr;
}

bool PageTable::ReleasePage(void* page_pointer) {
  __debugbreak();
  return false;
}

PageTable::PageEntry* PageTable::FindBackingPage(void* block) {
    #if 0
  for (auto i = 0; i < 12; i++) {
    auto& e = page_entries_[i];
    if (e.Contains(block)) {
      return &e;
    }
  }
  #endif
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
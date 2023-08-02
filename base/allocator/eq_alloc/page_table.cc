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

PageTable::PageTable(mem_size reserve_count) {
  // ReservePages(0, reserve_count);
  ReserveAddressSpace();
}

mem_size PageTable::ReserveAddressSpace() {
  auto *address_space = Reserve(nullptr, nullptr, 1_tib);
  first_page_ = reinterpret_cast<pointer_size>(address_space);
  return 1_tib;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags, mem_size* size_out) {
  ScopedSpinLock _;

  if (!freelist_) {
    void* block = Allocate(reinterpret_cast<void*>(first_page_.load()), ideal_page_size(),
                           page_flags,
                           0xFF, true);
    if (block && size_out)
      *size_out = ideal_page_size();
    return block;
  }

  // Pop a page from the freelist.
  FreeListEntry* freePage = freelist_;
  freelist_ = freelist_->next;

  if (size_out)
    *size_out = ideal_page_size();
  return static_cast<void*>(freePage);
}

bool PageTable::ReleasePage(void* page_pointer) {
  if (!page_pointer)
    return false;

  // Deallocate the page, return false if it fails.
  if (!DeAllocate(page_pointer))
    return false;

  // Insert the freed page into the freelist.

  // TODO: dont we have to adjust offset for freelist??!
  FreeListEntry* freePage = static_cast<FreeListEntry*>(page_pointer);
  freePage->next = freelist_;
  freelist_ = freePage;

  return true;
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
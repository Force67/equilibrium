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
  ReservePages(0, reserve_count);
}

mem_size PageTable::ReservePages(const pointer_size page_base,
                                 const mem_size page_reserve_count) {
  byte* preferred_address = page_base ? reinterpret_cast<byte*>(page_base) : nullptr;

  // const mem_size page_boundary = page_boundary_alignment();
  const mem_size page_boundary = 0;
  const mem_size page_size = ideal_page_size();

  byte* block = nullptr;
  if (page_boundary == 0) {
    // let the OS choose where in our 8tib address space to put the allocation
    if (preferred_address == nullptr) {
      block = Reserve(nullptr, nullptr, page_size);
      first_page_ = reinterpret_cast<pointer_size>(block);
      preferred_address = reinterpret_cast<byte*>(first_page_.load()) + page_size;
    }

    // no boundary, we simply can tack on the next page on the end of the current
    // page so we can allocate everything in one go
    const pointer_size total_adress_space = page_size * (page_reserve_count - 1);
    void* end = reinterpret_cast<void*>(static_cast<byte*>(preferred_address) +
                                        total_adress_space);
    block = Reserve(preferred_address, end, page_size * total_adress_space);
    __debugbreak();
  } else {
#if 0
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
#endif
  }

  // no pages were allocated
  if (!block)
    return 0;

  return page_reserve_count;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags, mem_size* size_out) {
  ScopedSpinLock _;

  if (!freelist_) {
    // No free pages, need to allocate a new one.
    // The preferred_address could be determined based on your specific memory layout
    // needs. For simplicity, let's assume `nullptr` here which indicates no specific
    // preference.
    void* block = Allocate(nullptr, ideal_page_size(), page_flags, 0xFF, true);
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
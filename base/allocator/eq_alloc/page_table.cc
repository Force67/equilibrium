// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

#include <base/allocator/virtual_memory.h>
#include "arch.h"
#include "compiler.h"

namespace base {

namespace {
constexpr mem_size kPageGrowByRatio = 2;

#if (OS_WIN)
constexpr u32 kIdealPageSize = eq_allocation_constants::kPageThreshold;
constexpr u32 kIdealAlignment = static_cast<u32>(1_mib);
#else
constexpr u32 kIdealPageSize = static_cast<u32>(64_kib);
constexpr u32 kIdealAlignment = static_cast<u32>(1_mib);
#endif
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
  first_page_ = reinterpret_cast<pointer_size>(
      base::VirtualMemoryReserve(nullptr, 1_tib));
  if (!first_page_)
    DEBUG_TRAP;
  return 1_tib;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags,
                             mem_size* size_out) {
  ScopedSpinLock _;
  (void)_;

  if (!freelist_) {
    byte* block =
        base::VirtualMemoryAllocate(reinterpret_cast<void*>(first_page_.load()),
                                    kIdealPageSize, page_flags, false);
    if (!block)
      DEBUG_TRAP;
    if (block) {
      ::memset(block, 0xFF, kIdealPageSize);

      if (size_out)
        *size_out = kIdealPageSize;
    }
    return block;
  }

  // Pop a page from the freelist.
  FreeListEntry* freePage = freelist_;
  freelist_ = freelist_->next;

  if (size_out)
    *size_out = kIdealPageSize;
  return static_cast<void*>(freePage);
}

mem_size PageTable::ReleasePage(void* page_pointer) {
  // Deallocate the page
  if (!page_pointer || !base::VirtualMemoryFree(page_pointer, kIdealPageSize))
    return 0u;
  // Insert the freed page into the freelist.
  // TODO: dont we have to adjust offset for freelist??!
  FreeListEntry* freePage = static_cast<FreeListEntry*>(page_pointer);
  freePage->next = freelist_;
  freelist_ = freePage;

  return kIdealPageSize;
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
  DEBUG_TRAP;
  return nullptr;
}

bool PageTable::FreeBlock(void* block) {
  // TODO: OS Free it
  PageEntry* entry = FindBackingPage(block);
  if (!entry)
    return false;

  entry->size = 0;
  return true;
}
}  // namespace base
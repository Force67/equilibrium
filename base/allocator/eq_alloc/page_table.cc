// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

#include <base/allocator/virtual_memory.h>
#include "arch.h"
#include "compiler.h"

#include <base/threading/spinning_mutex.h>
#include <base/threading/lock_guard.h>

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
}  // namespace

PageTable::PageTable(mem_size reserve_count)
    : page_reserve_count_(reserve_count) {
  ReserveAddressSpace();
}

PageTable::~PageTable() {
  // Free all pages
  PageEntry* entry = reinterpret_cast<PageEntry*>(metadata_page_.load());
  for (size_t i = 0; i < current_page_count_; i++) {
    if (!entry->available()) {
      base::VirtualMemoryFree(reinterpret_cast<void*>(entry->address),
                              entry->size);
    }
    entry++;
  }
  base::VirtualMemoryFree(reinterpret_cast<void*>(metadata_page_.load()), 0);
  // base::VirtualMemoryFree(reinterpret_cast<void*>(first_page_.load()), 0);
}

mem_size PageTable::ReserveAddressSpace() {
  address_space_ = reinterpret_cast<pointer_size>(
      base::VirtualMemoryReserve(nullptr, 1_tib));
  if (!address_space_)
    DEBUG_TRAP;
  // we also need to allocate a management page.
  if (!metadata_page_) {
    const auto memory_size = (sizeof(PageEntry) * page_reserve_count_);
    metadata_page_ = reinterpret_cast<pointer_size>(base::VirtualMemoryAllocate(
        nullptr, memory_size, PageProtectionFlags::RW));
    PageEntry* entry = reinterpret_cast<PageEntry*>(metadata_page_.load());
    for (size_t i = 0; i < page_reserve_count_; i++) {
      entry->flags = PageEntry::Flags::FREE;
      entry->address = address_space_ + (kIdealPageSize * i);
      entry++;
    }
  }
  return 1_tib;
}

PageTable::PageEntry* PageTable::FindFreePage() const {
  PageEntry* entry = reinterpret_cast<PageEntry*>(metadata_page_.load());
  for (size_t i = 0; i < page_reserve_count_; i++) {
    if (entry->available())
      return entry;
    entry++;
  }
  return nullptr;
}

PageTable::PageEntry* PageTable::FindBackingPage(void* block) {
  PageEntry* entry = reinterpret_cast<PageEntry*>(metadata_page_.load());
  for (size_t i = 0; i < page_reserve_count_; i++) {
    if (entry->address == reinterpret_cast<pointer_size>(block))
      return entry;
    entry++;
  }
  DEBUG_TRAP;
  return nullptr;
}

static byte* AllocatePage(void* at_address, PageProtectionFlags page_flags) {
  byte* block = reinterpret_cast<byte*>(base::VirtualMemoryAllocate(
      at_address, kIdealPageSize, page_flags, false));
  if (!block)
    DEBUG_TRAP;
  // whoa, we didn't get the address we wanted in the reserved block. Did you
  // call reserve?
  if (block != at_address)
    DEBUG_TRAP;
  ::memset(block, 0xFF, kIdealPageSize);  // not really ideal, but for safety
  return block;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags,
                             mem_size* size_out) {
  base::ScopedLockGuard<base::SpinningMutex> _;
  (void)_;
  // do we have any free pages?
  if (PageEntry* entry = FindFreePage()) {
    if (entry->address == 0u)
      DEBUG_TRAP;
    byte* block =
        AllocatePage(reinterpret_cast<void*>(entry->address), page_flags);
    entry->size = kIdealPageSize;
    entry->flags = PageEntry::Flags::IN_USE;
    entry->address = reinterpret_cast<pointer_size>(block);
    if (size_out)
      *size_out = entry->size;
    current_page_count_++;
    return static_cast<void*>(block);
  }
  return nullptr;
}

mem_size PageTable::ReleasePage(void* page_pointer) {
  base::ScopedLockGuard<base::SpinningMutex> _;
  // Deallocate the page memory
  if (!page_pointer || !base::VirtualMemoryFree(page_pointer, kIdealPageSize))
    return 0u;
  // Mark the page as free
  PageEntry* entry = FindBackingPage(page_pointer);
  entry->flags = PageEntry::Flags::FREE;
  current_page_count_--;
  return kIdealPageSize;
}
}  // namespace base
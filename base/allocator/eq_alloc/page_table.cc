// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

#include <base/allocator/virtual_memory.h>
#include "arch.h"
#include "compiler.h"

#include <base/threading/lock_guard.h>

#include <cstring>

namespace base {

namespace {
constexpr mem_size kPageGrowByRatio = 2;

}  // namespace

PageTable::PageTable(const mem_size space_size,
                     const mem_size page_size,
                     const mem_size reserve_count)
    : page_reserve_count_(reserve_count),
      address_space_(0),
      space_size_(0),
      page_size_(0),
      current_page_count_(0) {
  ReserveAddressSpace(space_size, page_size);
}

PageTable::~PageTable() {
  // just yeet the entire address space
  if (address_space_) {
    base::VirtualMemoryFree(reinterpret_cast<void*>(address_space_), space_size_);
  }
  // and the metadata page
  if (metadata_page_) {
    base::VirtualMemoryFree(reinterpret_cast<void*>(metadata_page_.load()), 0);
  }
}

bool PageTable::ReserveAddressSpace(const mem_size address_space_size,
                                    const mem_size page_size) {
  space_size_ = address_space_size;
  page_size_ = page_size;
  address_space_ = reinterpret_cast<pointer_size>(
      base::VirtualMemoryReserve(nullptr, address_space_size));
  if (!address_space_)
    DEBUG_TRAP;
  // we also need to allocate a management page.
  if (!metadata_page_) {
    const auto memory_size = (sizeof(PageEntry) * page_reserve_count_);
    metadata_page_ = reinterpret_cast<pointer_size>(
        base::VirtualMemoryAllocate(nullptr, memory_size, PageProtectionFlags::RW));
    PageEntry* entry = reinterpret_cast<PageEntry*>(metadata_page_.load());
    for (size_t i = 0; i < page_reserve_count_; i++) {
      entry->flags = PageEntry::Flags::FREE;
      entry->address = address_space_ + (page_size * i);
      entry++;
    }
  }
  return true;
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

static byte* AllocatePage(void* at_address,
                          mem_size page_size,
                          PageProtectionFlags page_flags) {
  byte* block = reinterpret_cast<byte*>(
      base::VirtualMemoryAllocate(at_address, page_size, page_flags, false));
  if (!block)
    DEBUG_TRAP;
  // whoa, we didn't get the address we wanted in the reserved block. Did you
  // call reserve?
  if (block != at_address)
    DEBUG_TRAP;
  memset(block, 0xFF, page_size);  // not really ideal, but for safety
  return block;
}

void* PageTable::RequestPage(PageProtectionFlags page_flags, mem_size* size_out) {
  base::NonOwningScopedLockGuard _(lock_);
  (void)_;
  // do we have any free pages?
  if (PageEntry* entry = FindFreePage()) {
    if (entry->address == 0u)
      DEBUG_TRAP;
    byte* block =
        AllocatePage(reinterpret_cast<void*>(entry->address), page_size_, page_flags);
    entry->size = page_size_;
    entry->flags = PageEntry::Flags::IN_USE;
    entry->address = reinterpret_cast<pointer_size>(block);
    if (size_out)
      *size_out = entry->size;
    current_page_count_++;
    return static_cast<void*>(block);
  }
  return nullptr;
}

void* PageTable::RequestPages(mem_size count,
                              PageProtectionFlags page_flags,
                              mem_size* size_out) {
  if (count == 0)
    return nullptr;
  if (count == 1)
    return RequestPage(page_flags, size_out);

  base::NonOwningScopedLockGuard _(lock_);
  (void)_;
  PageEntry* entries = reinterpret_cast<PageEntry*>(metadata_page_.load());

  // find a contiguous run of 'count' free page entries
  for (mem_size i = 0; i + count <= page_reserve_count_; i++) {
    bool all_free = true;
    for (mem_size j = 0; j < count; j++) {
      if (!entries[i + j].available()) {
        all_free = false;
        i += j;  // skip past the occupied entry
        break;
      }
    }
    if (!all_free)
      continue;

    // commit every page in the run
    byte* base = nullptr;
    for (mem_size j = 0; j < count; j++) {
      auto& e = entries[i + j];
      byte* page = AllocatePage(reinterpret_cast<void*>(e.address), page_size_, page_flags);
      if (j == 0)
        base = page;
      e.size = page_size_;
      e.flags = PageEntry::Flags::IN_USE;
    }
    current_page_count_ += count;
    if (size_out)
      *size_out = page_size_ * count;
    return base;
  }
  return nullptr;
}

mem_size PageTable::ReleasePages(void* address, mem_size count) {
  mem_size total = 0;
  byte* addr = static_cast<byte*>(address);
  for (mem_size i = 0; i < count; i++) {
    total += ReleasePage(addr + (page_size_ * i));
  }
  return total;
}

mem_size PageTable::ReleasePage(void* page_pointer) {
  base::NonOwningScopedLockGuard _(lock_);
  // Deallocate the page memory
  if (!page_pointer || !base::VirtualMemoryFree(page_pointer, page_size_))
    return 0u;
  // Mark the page as free
  PageEntry* entry = FindBackingPage(page_pointer);
  entry->flags = PageEntry::Flags::FREE;
  current_page_count_--;
  return page_size_;
}
}  // namespace base
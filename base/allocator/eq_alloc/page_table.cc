// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

#include <base/allocator/virtual_memory.h>
#include "arch.h"
#include "compiler.h"

#include <base/threading/lock_guard.h>

namespace base {

namespace {
constexpr mem_size kPageGrowByRatio = 2;

}  // namespace

PageTable::PageTable(const mem_size space_size,
                     const mem_size page_size,
                     const mem_size reserve_count)
    : metadata_page_(0),
      page_reserve_count_(reserve_count),
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
    const mem_size metadata_size = sizeof(PageEntry) * page_reserve_count_;
    base::VirtualMemoryFree(reinterpret_cast<void*>(metadata_page_.load()),
                            metadata_size);
  }
}

bool PageTable::ReserveAddressSpace(const mem_size address_space_size,
                                    const mem_size page_size) {
  space_size_ = address_space_size;
  page_size_ = page_size;

  // over-reserve so we can align the usable region to page_size.
  // this guarantees (address_space_ & (page_size-1)) == 0, enabling
  // single-AND mask lookups from any interior pointer to its page base.
  const mem_size extra = page_size - 1;
  byte* raw =
      base::VirtualMemoryReserve(nullptr, address_space_size + extra);
  if (!raw)
    DEBUG_TRAP;
  address_space_ =
      (reinterpret_cast<pointer_size>(raw) + extra) & ~extra;
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

static byte* CommitRange(void* at_address,
                         mem_size total_size,
                         PageProtectionFlags page_flags) {
  byte* block = reinterpret_cast<byte*>(
      base::VirtualMemoryAllocate(at_address, total_size, page_flags, false));
  if (!block)
    DEBUG_TRAP;
  if (block != at_address)
    DEBUG_TRAP;
  // mmap(MAP_ANONYMOUS) returns zeroed pages — no memset needed
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
        CommitRange(reinterpret_cast<void*>(entry->address), page_size_, page_flags);
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

    // commit the entire contiguous range in a single syscall
    byte* base = CommitRange(reinterpret_cast<void*>(entries[i].address),
                             page_size_ * count, page_flags);
    for (mem_size j = 0; j < count; j++) {
      entries[i + j].size = page_size_;
      entries[i + j].flags = PageEntry::Flags::IN_USE;
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
  // Drop the physical storage without releasing the page table's reservation.
  if (!page_pointer || !base::VirtualMemoryDecommit(page_pointer, page_size_))
    return 0u;
  // Mark the page as free
  PageEntry* entry = FindBackingPage(page_pointer);
  entry->flags = PageEntry::Flags::FREE;
  current_page_count_--;
  return page_size_;
}
}  // namespace base

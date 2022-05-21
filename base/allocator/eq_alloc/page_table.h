// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/arch.h>
#include <base/containers/linked_list.h>
#include <base/memory/memory_literals.h>

namespace base {

// page table only manages pages, not blocks within these pages.
class PageTable {
 public:
  PageTable();

  static u32 ideal_page_size();

  void* RequestPage();
  void* RequestPage(size_t& size_out);

 private:
  void ReservePages(const pointer_size page_base, const mem_size count);

  byte* Reserve(void* preferred, size_t block_size);

  // initialize_with should be an optional...
  void* Allocate(void* preferred_address,
                 size_t block_size,
                 byte initalize_with,
                 bool use_initialize,
                 bool read_only = false);
  bool Free(void* address);

 private:
  struct PageEntry {
    u32 address_;
    u32 size_;
    // TODO: when allocating within here, store a ref count?

    bool Contains(u32 block) const {
      return address_ >= block && block <= (address_ + size_);
    }
  };

  byte* DecompressPointer(const PageEntry& e) {
    DCHECK(page_table_base_);
    return reinterpret_cast<byte*>(page_table_base_ + e.address_);
  }

  u32 CompressPointer(void* block) {
    DCHECK(page_table_base_);
    return u32(reinterpret_cast<pointer_size>(block) - page_table_base_);
    // return u32(reinterpret_cast<pointer_size>(block) & 0xFFFFFFFF);
  }

  PageEntry* FindBackingPage(void* address);

  // In order to reduce memory usage as much as possible, we just store indices into
  // the different pages starting from this address by default we pre-reserve
  // kPagePrereserveCount starting from page_base_address_
  bool zero_pages_ = true;
  pointer_size page_table_base_{0};
  PageEntry page_entries_[12]{};
};
}  // namespace base
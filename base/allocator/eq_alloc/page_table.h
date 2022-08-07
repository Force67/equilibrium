// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/atomic.h>
#include <base/check.h>
#include <base/containers/linked_list.h>
#include <base/memory/memory_literals.h>

#include <base/allocator/page_protection.h>

namespace base {
using namespace memory_literals;

// page table only manages pages, not blocks within these pages.
class PageTable {
 public:
  PageTable();

  struct Options {
    bool zero_pages{true};
  };

  // os page size
  static u32 current_page_size();
  // the ideal page size, we will use
  static u32 ideal_page_size();
  // where we will align the pages to, for instance if this returns 1mib, each 64kib
  // page would be allocated at 1mib steps
  static u32 page_boundary_alignment();

  void* RequestPage(PageProtectionFlags page_flags,
                    mem_size* size_optional_out = nullptr);

  bool ReleasePage(void* address);

 private:
  mem_size ReservePages(const pointer_size page_base, const mem_size count);

  byte* Reserve(void* preferred, mem_size block_size);

  // initialize_with should be an optional...
  void* Allocate(void* preferred_address,
                 mem_size block_size,
                 base::PageProtectionFlags,
                 byte initalize_with,
                 bool use_initialize);

  bool Free(void* address);

 private:
  // a single page can only ever be owned by one allocator.
  // we cant store the page entries in the pages themselves because this could get
  // quite wasteful with 64kib, so we need to maintain the records in a list of small
  // pages
  struct PageEntry {
    pointer_size address;
    mem_size size;  // refcount

    inline bool Contains(pointer_size block) const {
      return address >= block && block <= (address + size);
    }

    inline bool Contains(const void* pointer) const {
      return Contains(reinterpret_cast<pointer_size>(pointer));
    }

    void* pointer() const { return reinterpret_cast<void*>(address); }
  };
  static_assert(sizeof(PageEntry) == 2 * sizeof(pointer_size),
                "PageEntry alignment is invalid");

  // do we even need a size parameter if every page is 64k?
  // do we even need an address if they are a continuous array? (e.g aligned to a
  // 1mib boundary?)

  // TODO: set class for these schenanigans?
  PageEntry* FindBackingPage(void* address);

  Options options_{};

  // https://github.com/SerenityOS/serenity/blob/master/Kernel/Memory/PageDirectory.cpp
  //  pageDictionary

  base::Atomic<pointer_size> first_page_;
  base::Atomic<pointer_size> last_page_;
  base::Atomic<pointer_size> current_page_;

  base::Atomic<mem_size> metadata_page_count_;
  base::Atomic<mem_size> current_page_count_;
};
}  // namespace base
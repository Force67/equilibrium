// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/atomic.h>
#include <base/check.h>
#include <base/containers/linked_list.h>
#include <base/memory/memory_literals.h>

#include <base/allocator/memory_range.h>
#include <base/allocator/virtual_memory.h>

namespace base {
using namespace memory_literals;

// page table only manages pages, not blocks within these pages.
class PageTable {
 public:
  explicit PageTable(mem_size reserve_count);

  struct Options {
    bool zero_pages{true};
  };
  void* RequestPage(PageProtectionFlags page_flags,
                    mem_size* size_optional_out = nullptr);

  // returns the page size or 0 if the page was not found
  mem_size ReleasePage(void* address);

  uintptr_t PageOffset(void* address) {
    auto b = reinterpret_cast<uintptr_t>(address);
    DCHECK(first_page_ != 0, "First page not set");
    //DCHECK(b >= first_page_, "Page out of bounds");
    return b - first_page_;
  }

 private:
  mem_size ReserveAddressSpace();
  
  bool FreeBlock(void* block_address);

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

  struct FreeListEntry {
    FreeListEntry* next;
  };
  FreeListEntry* freelist_ = nullptr;

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
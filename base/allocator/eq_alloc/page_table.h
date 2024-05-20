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

#include <base/enum_traits.h>

namespace base {
using namespace memory_literals;

// page table only manages pages, not blocks within these pages.
class PageTable {
 public:
  explicit PageTable(mem_size reserve_count);
  ~PageTable();

  struct Options {
    bool zero_pages{true};
  };
  void* RequestPage(PageProtectionFlags page_flags,
                    mem_size* size_optional_out = nullptr);

  // returns the page size or 0 if the page was not found
  mem_size ReleasePage(void* address);

  uintptr_t PageOffset(void* address) {
    auto b = reinterpret_cast<uintptr_t>(address);
    DCHECK(address_space_ != 0, "First page not set");
    // DCHECK(b >= first_page_, "Page out of bounds");
    return b - address_space_;
  }

 private:
  mem_size ReserveAddressSpace();

 private:
  // do we even need a size parameter if every page is 64k?
  // do we even need an address if they are a continuous array? (e.g aligned to
  // a 1mib boundary?)
  // metadata page layout:
  // +-------------------------------------------------------------------+
  // | growing forwards: PageEntry 1 | PageEntry 2 | PageEntry 3 | ...   |
  // +-------------------------------------------------------------------+
  struct PageEntry {
    enum Flags : u32 {
      IN_USE = 1 << 0,
      FREE = 1 << 1,
    };
    u32 flags{Flags::IN_USE};
    u32 size;
    pointer_size address;

    PageEntry(pointer_size address, mem_size size)
        : address(address), size(size) {}

    inline bool Contains(pointer_size block) const {
      return address >= block && block <= (address + size);
    }

    inline bool Contains(const void* pointer) const {
      return Contains(reinterpret_cast<pointer_size>(pointer));
    }

    void* pointer() const { return reinterpret_cast<void*>(address); }

    inline bool available() const { return flags & Flags::FREE; }
  };
  static_assert(sizeof(PageEntry) == 2 * sizeof(pointer_size),
                "PageEntry alignment is invalid");

  base::Atomic<pointer_size> metadata_page_;
  base::Atomic<mem_size> page_reserve_count_;

  PageEntry* FindFreePage() const;

  // TODO: set class for these schenanigans?
  PageEntry* FindBackingPage(void* address);

  Options options_{};

  // https://github.com/SerenityOS/serenity/blob/master/Kernel/Memory/PageDirectory.cpp
  //  pageDictionary

  // page base pointers
  // overall page layout:
  // +-----------------------Page-Space (1-Tib)--------------------------+
  // | Page 1 | Page 2 | Page 3 | ... | Reserved | Reserved | Reserved   |
  // +-------------------------------------------------------------------+
  pointer_size address_space_;
  mem_size current_page_count_;
};
}  // namespace base
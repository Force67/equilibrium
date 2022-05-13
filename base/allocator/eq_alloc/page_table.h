// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/containers/linked_list.h>

namespace base {

// page table only manages pages, not blocks within these pages.
class PageTable {
 public:
  PageTable() = default;

  static constexpr size_t kPageLimit = 12;

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

    float CalculateFullnessRatio() const { return 0.f; }
  };

  PageEntry* TakeFreePageEntry();
  PageEntry* FindBackingPage(void* address);

  // very naive initial implementation

  // indicides within this backing buffer also indicate where the entries are
  // located?
  PageEntry entries_[kPageLimit]{};
};
}  // namespace base
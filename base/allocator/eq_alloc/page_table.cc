// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>

namespace base {

PageTable::PageTable() {
  PrereserveInitialPages();
}

void PageTable::PrereserveInitialPages() {
  byte* preferred =
      page_table_base_ ? reinterpret_cast<byte*>(page_table_base_) : nullptr;

  // reserve the initial chunk in one go.
  byte* block = static_cast<byte*>(
      Reserve(preferred, ideal_page_size() * kPagePrereserveAttemptCount));
  if (!block)
    return;
  if (!page_table_base_)
    page_table_base_ = reinterpret_cast<pointer_size>(block);

  if (!page_table_base_)
    page_table_base_ = reinterpret_cast<pointer_size>(block);

  for (auto i = 0; i < kPagePrereserveAttemptCount; i++) {
    auto& e = page_entries_[i];

    if constexpr (sizeof(u32) != sizeof(pointer_size)) {
      // 64 bit
      e.address_ = CompressPointer(block);
    } else {
      e.address_ = u32(reinterpret_cast<pointer_size>(block));
    }

    if (DecompressPointer(e) != block) {
      __debugbreak();
    }

    block += ideal_page_size();
    // no point in setting the size, it indicates the free'ness
  }
}

void PageTable::ReservePagesOnebyOne() {
  byte* preferred =
      page_table_base_ ? reinterpret_cast<byte*>(page_table_base_) : nullptr;
  for (size_t i = 0; i < kPagePrereserveAttemptCount; i++) {
    void* block = Reserve(static_cast<void*>(preferred), ideal_page_size());
    if (!block)
      return;

    if (!page_table_base_)
      page_table_base_ = reinterpret_cast<pointer_size>(block);
    if (!preferred)
      preferred = static_cast<byte*>(block);

    if (static_cast<byte*>(block) != preferred) {
      DEBUG_TRAP;
    }

    if constexpr (sizeof(u32) != sizeof(pointer_size)) {
      // 64 bit
      page_entries_[i].address_ = CompressPointer(block);
    } else {
      page_entries_[i].address_ = u32(reinterpret_cast<pointer_size>(block));
    }
    // we dont let the system know about the suggested sizes however
    page_entries_[i].size_ = 0;

    preferred += ideal_page_size();
  }
}

void* PageTable::RequestPage() {
  // enter lock...
  for (auto i = 0; i < kPagePrereserveAttemptCount; i++) {
    auto& e = page_entries_[i];
    if (e.size_ != 0)
      continue;

    if (void* mem = Allocate(DecompressPointer(e), ideal_page_size(), 0, zero_pages_,
                             false)) {
      // this marks the block as 'in-use', so we only do it on success
      e.size_ = ideal_page_size();
      return mem;
    }
  }

  return nullptr;
}

void* PageTable::RequestPage(size_t& size) {
  // enter lock...
  for (auto i = 0; i < kPagePrereserveAttemptCount; i++) {
    auto& e = page_entries_[i];
    if (e.size_ != 0)
      continue;

    // for debugging reasons we fill each new page with 0xFF to detect unitialized
    // memory more easily
    if (void* mem =
            Allocate(DecompressPointer(e), ideal_page_size(), 0xFF, true, false)) {
      // this marks the block as 'in-use', so we only do it on success
      e.size_ = ideal_page_size();
      size = ideal_page_size();
      return mem;
    }
  }

  return nullptr;
}

PageTable::PageEntry* PageTable::FindBackingPage(void* block) {
  u32 compressed_ptr = CompressPointer(block);

  for (auto i = 0; i < kPagePrereserveAttemptCount; i++) {
    auto& e = page_entries_[i];
    if (e.Contains(compressed_ptr)) {
      return &e;
    }
  }

  return nullptr;
}

bool PageTable::Free(void* block) {
  // TODO: OS Free it
  PageEntry* entry = FindBackingPage(block);
  if (!entry)
    return false;

  entry->size_ = 0;
  return true;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_memory_router.h>

#include <new>  // for placement new

namespace base {
PageTable* EQMemoryRouter::page_table() {
  if (!page_table_data_[0]) {
    PageTable* table = new (page_table_data_) PageTable();
    InitializeAllocators(table);
    return table;
  }
  return reinterpret_cast<PageTable*>(&page_table_data_[0]);
}

void EQMemoryRouter::InitializeAllocators(PageTable* page_table) {
  static BucketAllocator bucket;
  allocators_[0] = &bucket;
}
}  // namespace base
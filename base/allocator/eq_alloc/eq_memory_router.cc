// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_allocator.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/heap_allocator.h>

#include <base/allocator/eq_alloc/eq_memory_router.h>

#include <new>  // for placement new

namespace base {

namespace {
alignas(BucketAllocator) byte
    bucket_allocator_storage[sizeof(BucketAllocator)]{};
alignas(PageAllocator) byte page_allocator_storage[sizeof(PageAllocator)]{};
alignas(HeapAllocator) byte heap_allocator_storage[sizeof(HeapAllocator)]{};
#if (OS_WIN)
constexpr u32 kIdealPageSize = eq_allocation_constants::kPageThreshold;
constexpr u32 kIdealAlignment = static_cast<u32>(1_mib);
#else
constexpr u32 kIdealPageSize = static_cast<u32>(64_kib);
constexpr u32 kIdealAlignment = static_cast<u32>(1_mib);
#endif
}  // namespace

// TODO: maybe refactor this in some complex obj init instantiate shit
PageTable* EQMemoryRouter::page_table() {
  if (!page_table_data_[0]) {
    // with 5 pages?
    PageTable* table = new (&page_table_data_[sizeof(UINT_MAX)])
        PageTable(1_tib /*This should be a base compile opt later on..*/,
                  kIdealPageSize, 5);
    InitializeAllocators(*table);
    // tombstone this so never ever ever there can be more than one pagetable.
    *reinterpret_cast<uint32_t*>(&page_table_data_[0]) = UINT32_MAX;
    return table;
  }
  return reinterpret_cast<PageTable*>(&page_table_data_[sizeof(UINT_MAX)]);
}

void EQMemoryRouter::InitializeAllocators(PageTable& page_table) {
  // placement new
  allocators_[AllocatorID::kBucketAllocator] =
      new (bucket_allocator_storage) BucketAllocator(page_table);
  allocators_[AllocatorID::kPageAllocator] =
      new (page_allocator_storage) PageAllocator(page_table);
  allocators_[AllocatorID::kHeapAllocator] =
      new (heap_allocator_storage) HeapAllocator();
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/eq_alloc/page_allocator.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/heap_allocator.h>

#include <base/allocator/eq_alloc/eq_memory_router.h>

#include <new>  // for placement new

namespace base {

namespace {
alignas(BucketAllocator) byte bucket_allocator_storage[sizeof(BucketAllocator)]{};
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

// TODO: refactor this into a proper initialization sequence.
PageTable* EQMemoryRouter::page_table() {
  auto* table = reinterpret_cast<PageTable*>(&page_table_data_[0]);

  // The fast path, once the table is published.
  if (page_table_state_.load(memory_order_acquire) == kInitialized)
    return table;

  // Exactly one thread wins the claim and constructs; a test-then-construct
  // would let two concurrent first allocations both build a page table over
  // the same storage and hand out two sets of allocators.
  u32 expected = kUninitialized;
  if (page_table_state_.compare_exchange_strong(expected, kInitializing,
                                                memory_order_acq_rel,
                                                memory_order_acquire)) {
    // 8192 page entries = 512 MiB addressable with 64 KiB pages.
    // metadata overhead: 8192 * 16 = 128 KiB.
    new (table) PageTable(
        1_tib /*This should be a base compile opt later on..*/, kIdealPageSize, 8192);
    InitializeAllocators(*table);
    page_table_state_.store(kInitialized, memory_order_release);
    return table;
  }

  // The losers wait for the winner to publish. There is no yield primitive
  // this far down in the allocator, and the window is one PageTable
  // construction long.
  while (page_table_state_.load(memory_order_acquire) != kInitialized) {
  }
  return table;
}

void EQMemoryRouter::InitializeAllocators(PageTable& page_table) {
  // placement-new the allocator instances into preallocated storage.
  allocators_[AllocatorID::kBucketAllocator] =
      new (bucket_allocator_storage) BucketAllocator(page_table);
  allocators_[AllocatorID::kPageAllocator] =
      new (page_allocator_storage) PageAllocator(page_table);
  allocators_[AllocatorID::kHeapAllocator] =
      new (heap_allocator_storage) HeapAllocator(page_table);
}
}  // namespace base
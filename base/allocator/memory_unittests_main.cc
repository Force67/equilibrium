// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// Defines a special entry point for memory related unit testing.

#include <base/check.h>

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <allocator/eq_alloc/page_table.h>
#include <allocator/eq_alloc/bucket_allocator.h>
#include <allocator/memory_coordinator.h>

#include <cstdio>

using namespace base;

// bugcheck isnt available in this context
#define ENSURE_THAT(expression) if (!(expression)) { DEBUG_TRAP; }

void EQPageTableTest1() {
  PageTable table(0xFFFF * 2, 0xFFFF, 2);
  ENSURE_THAT(table.page_size() == 0xFFFF);
 // u32 size = PageTable::current_page_size();

  void* page = table.RequestPage(base::PageProtectionFlags::RW);
  ENSURE_THAT(page);
}

void TrackerTest1() {
  auto& tracker = base::memory_tracker();
  // demonstates the use of the tracker for debugging
  char* obj = new char[512];
  mem_size store = tracker.memory_sizes[kGeneralMemory].load();
  // somehow this is thread local now?
  ENSURE_THAT(store == 512);

  delete[] obj;
  ENSURE_THAT(tracker.memory_sizes[kGeneralMemory] == 0);
}

void TrackerTest2() {
  auto& tracker = base::memory_tracker();

  void* x = base::memory_coordinator().Allocate(10);
  mem_size store = tracker.memory_sizes[kGeneralMemory];
  ENSURE_THAT(store == 10);

  x = base::memory_coordinator().ReAllocate(x, 20);
  ENSURE_THAT(tracker.memory_sizes[kGeneralMemory] == 20);

  x = base::memory_coordinator().ReAllocate(x, 15);
  ENSURE_THAT(tracker.memory_sizes[kGeneralMemory] == 15);

  base::memory_coordinator().Free(x);
  ENSURE_THAT(tracker.memory_sizes[kGeneralMemory] == 0);
}

void TestMemoryCategories() {
  for (auto i = 0; i < 254; i++) {
    base::AddMemoryCategory("TEST");
  }

  auto& tracker = base::memory_tracker();
}

void BucketAllocatorTest_Allocate() {
  base::PageTable page_table(0xFFFF * 2, 0xFFFF, 2);
  base::BucketAllocator allocator(page_table);

  void* block = allocator.Allocate(10, 8);
  ENSURE_THAT(block != nullptr);
  mem_size allocated_size = allocator.QueryAllocationSize(block);
  ENSURE_THAT(allocated_size >= 10);
  ENSURE_THAT(allocator.Free(block) == 10);

  // now lets allocate a bunch of blocks:
  for (int i = 0; i < 100; i++) {
	block = allocator.Allocate(10, 8);
	ENSURE_THAT(block != nullptr);
	allocated_size = allocator.QueryAllocationSize(block);
	ENSURE_THAT(allocated_size >= 10);
  }
}

void BucketAllocatorTest_ReAllocate_Grow() {
  base::PageTable page_table(0xFFFF * 2, 0xFFFF, 2);
  BucketAllocator allocator(page_table);

  void* block = allocator.Allocate(10, 8);
  ENSURE_THAT(block != nullptr);
  block = allocator.ReAllocate(block, 20, 8);
  ENSURE_THAT(block != nullptr);
  mem_size allocated_size = allocator.QueryAllocationSize(block);
  ENSURE_THAT(allocated_size >= 20);
  ENSURE_THAT(allocator.Free(block) == 20);
}


int main() {
#if (BASE_USE_EQ_ALLOCATOR)
  EQPageTableTest1();
#endif
  // explicitly test the bucket allocator
  BucketAllocatorTest_Allocate();
  BucketAllocatorTest_ReAllocate_Grow();

  // tracker
  TrackerTest1();
  TrackerTest2();
  TestMemoryCategories();

  return 0;
}
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// Defines a special entry point for memory related unit testing.

#include <base/check.h>

#include <allocator/eq_alloc/page_table.h>
#include <allocator/memory_coordinator.h>

#include <cstdio>

using namespace base;

void EQPageTableTest1() {
  PageTable table;
  u32 size = PageTable::current_page_size();

  void* page = table.RequestPage(base::PageProtectionFlags::RW);
  BUGCHECK(page);

}

void TrackerTest1() {
  // demonstates the use of the tracker for debugging
  void* obj = new char[512];
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 512);

  delete[] obj;
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 0);
}

void TrackerTest2() {
  void* x = base::memory_coordinator().Allocate(10);
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 10);

  x = base::memory_coordinator().ReAllocate(x, 20);
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 20);

  x = base::memory_coordinator().ReAllocate(x, 15);
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 15);

  base::memory_coordinator().Free(x);
  BUGCHECK(base::memory_tracker().memory_sizes[kGeneralMemory] == 0);
}

int main() {
#if (BASE_USE_EQ_ALLOCATOR)
  EQPageTableTest1();
#endif

  TrackerTest1();
  TrackerTest2();

  return 0;
}
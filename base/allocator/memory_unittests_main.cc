// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// Defines a special entry point for memory related unit testing.

#include <base/check.h>
#include <allocator/memory_coordinator.h>

using namespace base;

void Test1() {
  // demonstates the use of the tracker for debugging
  void* obj = new char[512];
  BUGCHECK(base::memory_coordinator().tracker().memory_sizes[kGeneralMemory] == 512);

  delete[] obj;
  BUGCHECK(base::memory_coordinator().tracker().memory_sizes[kGeneralMemory] == 0);
}

int main() {
  Test1();

  return 0;
}
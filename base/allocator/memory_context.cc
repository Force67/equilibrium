// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/thread.h>
#include <base/allocator/memory_context.h>
#include <base/allocator/memory_coordinator.h>

namespace base {
static thread_local MemoryCategory current_context{MemoryCategory::kMain};

MemoryScope::MemoryScope(MemoryCategory type, bool force) {
  thread_index_ = base::GetCurrentThreadIndex();

  // Enter the new allocator/pool instance.
  if (current_context != type) {
    Enter(type);
  }
}

MemoryScope::~MemoryScope() {
  Enter(prev_context_);
}

void MemoryScope::Enter(MemoryCategory type) {
  prev_context_ = current_context;
  current_context = type;

  //memory_coordinator().SwitchThreadAllocator();
}
}  // namespace base
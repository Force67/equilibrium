// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/threading/thread.h>
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/eq_alloc/memory_context.h>

namespace base {
static thread_local u32 current_memory_id{0};

MemoryScope::MemoryScope(u32 id, bool force) {
  thread_index_ = base::GetCurrentThreadIndex();
  // Enter the new allocator/pool instance.
  if (current_memory_id != id) {
    Enter(id);
  }
}

MemoryScope::~MemoryScope() {
  Enter(prev_context_);
}

void MemoryScope::Enter(u32 id) {
  prev_context_ = current_memory_id;
  current_memory_id = id;

  //memory_coordinator().SwitchThreadAllocator();
}
}  // namespace base
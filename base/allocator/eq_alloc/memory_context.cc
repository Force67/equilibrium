// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#if defined(OS_WIN)
#include <Windows.h>
#endif

#include <base/allocator/memory_coordinator.h>
#include <base/allocator/eq_alloc/memory_context.h>

namespace base {
namespace {
#if defined(OS_WIN)

// we are not using base::GetCurrentThreadIndex to avoid depending on it, since it's
// source file uses higher level language constructs that allocate
u32 Memory_GetCurrentThreadIndex() {
  return ::GetCurrentThreadId();
}
#endif

thread_local u32 current_memory_id{0};
}  // namespace

MemoryScope::MemoryScope(u32 id, bool force) {
  thread_index_ = Memory_GetCurrentThreadIndex();
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

  // memory_coordinator().SwitchThreadAllocator();
}
}  // namespace base
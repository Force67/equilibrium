// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/thread.h>
#include <base/allocator/memory_context.h>

namespace base {
static thread_local MemoryCategory current_context{MemoryCategory::kMain};

MemoryScope::MemoryScope(MemoryCategory type, bool force) {
  thread_index_ = GetCurrentThreadIndex();
  // switch thread context
  if (current_context != type) {
    Enter(current_context);
  }
}

MemoryScope::~MemoryScope() {
  Enter(prev_context_);
}

void MemoryScope::Enter(MemoryCategory type) {
  prev_context_ = current_context;
  current_context = type;
}

}  // namespace base
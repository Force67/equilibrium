// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "allocation_scope.h"

namespace base {
namespace {
// we use -1 to signal: no current override set.
thread_local i16 current_allocator_id{AllocationScope::NoOverride};
}  // namespace

void AllocationScope::Enter(allocator_handle new_id) {
  previous_allocator_ = current_allocator_id;
  current_allocator_id = static_cast<allocator_id>(new_id);
  // STILL We should check the thread id here, if the closure was somehow moved onto
  // a different thread!!!!!!!!
}

AllocationScope::allocator_handle AllocationScope::current_allocator() {
  return current_allocator_id;
}
}  // namespace base

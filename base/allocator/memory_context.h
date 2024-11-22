// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>

namespace base {

// for optimization reasons at most 255 allocators are supported
using allocator_id = u8;

// Memoryscopes allow you to override the allocator for a given scope
// e.g
// void f() { Memoryscope _(12); auto x = new char[512], } <-- Memory is
// provided and owned by allocator 12. As soon as they run out of scope they
// return to the formerly set allocator
class MemoryScope {
 public:
  // we use the custom type here, as we want to store -1 for the default beh,
  // but also need to fit the whole range of 'allocator_id'
  using allocator_handle = i16;

  // -1 here really means: return to the default eq_alloc behaviour, and is the
  // default id set on every thread
  static constexpr allocator_handle NoOverride = -1;

  explicit inline MemoryScope(allocator_id id) {
    DCHECK(id > 0 && id <= 255, "Invalid ID?");
    // Enter the new allocator/pool instance.
    if (current_allocator() != id) {
      Enter(static_cast<allocator_handle>(id));
    }
  }

  inline ~MemoryScope() { Enter(previous_allocator_); }

  // memory substem is responsible for polling this state.
  static allocator_handle current_allocator();

 private:
  void Enter(allocator_handle id);

 private:
  i16 previous_allocator_{};
};
}  // namespace base
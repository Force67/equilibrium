// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

class MemoryScope {
 public:
  MemoryScope(u32 id, bool force = false);
  ~MemoryScope();

 private:
  void Enter(u32 id);

 private:
  u32 prev_context_;
  u32 thread_index_;
};
}  // namespace base
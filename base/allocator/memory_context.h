// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

enum class MemoryCategory : u32 {
  kMain,
};

class MemoryScope {
 public:
  MemoryScope(MemoryCategory type, bool force = false);
  ~MemoryScope();

 private:
  void Enter(MemoryCategory type);

 private:
  MemoryCategory prev_context_;
  MemoryCategory cur_context_;
  u32 thread_index_;
};
}  // namespace base
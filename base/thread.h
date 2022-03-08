// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <base/arch.h>

namespace base {

class Thread {
 public:
  enum class Priority { kLow, kNormal, kHigh, kVeryHigh };


 private:
  void* handle_ = nullptr;
};

u32 GetCurrentThreadIndex();

void SetCurrentThreadPriority(Thread::Priority new_priority);
void SetCurrentThreadName(const char* name);

}  // namespace base
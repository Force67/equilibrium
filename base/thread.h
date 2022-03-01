// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <base/arch.h>

namespace base {

enum class ThreadPriority : u32 {
  Low = 0,
  Normal = 1,
  High = 2,
  VeryHigh = 3,
};

u32 GetCurrentThreadIndex();

void SetCurrentThreadPriority(ThreadPriority new_priority);
void SetCurrentThreadName(const char* name);

}  // namespace base
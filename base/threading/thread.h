// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/strings/xstring.h>

namespace base {

class Thread {
 public:
  enum class Priority { kLow, kNormal, kHigh, kVeryHigh };
  using Handle = void*;

  explicit Thread(const base::StringRef name, const Priority = Priority::kNormal);

  virtual u32 Run();

  bool good() const { return handle_; }

  void SetName(const base::StringRef name);
  void ApplyName();

  inline void SetPrio(const Thread::Priority prio);
  inline Priority GetPrio() const;

 private:
  Handle Spawn();

 private:
  // raw underlying implementation defined handle
  Handle handle_ = nullptr;
  u32 parent_thread_index_;
  base::String thread_name_;
};

// core threading primitives
u32 GetCurrentThreadIndex();
Thread::Handle GetCurrentThreadHandle();

bool SetThreadName(Thread::Handle, const char* name);
inline bool SetCurrentThreadName(const char* name) {
  return SetThreadName(GetCurrentThreadHandle(), name);
}

void SetThreadPriority(Thread::Handle, Thread::Priority new_priority);
const Thread::Priority GetThreadPriority(Thread::Handle);
const u32 GetNativeThreadPriority(Thread::Handle);
}  // namespace base
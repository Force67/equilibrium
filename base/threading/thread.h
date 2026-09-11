// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/export.h>
#include <base/strings/string_ref.h>
#include <base/functional/function.h>

namespace base {

class BASE_EXPORT Thread {
 public:
  virtual ~Thread() = default;

  union Handle {
    void* handle_;
    pointer_size pthread_;
  };

  enum class Priority { kLow, kNormal, kHigh, kVeryHigh };

  explicit Thread(const base::StringRef name,
                  base::Function<void()> functor,
                  bool start_now = false,
                  const Priority = Priority::kNormal);

  bool Start(const Priority);

  virtual u32 Run();

  // since both are same size anyway, we can use the same type
  bool good() const { return handle_data_.handle_ != nullptr; }

  void SetName(const base::StringRef name);

  // call this from the new thread index
  void ApplyName();

  inline void SetPrio(const Thread::Priority prio);
  inline Priority GetPrio() const;

 private:
  Handle Spawn();

 private:
  Handle handle_data_{};
  u32 parent_thread_index_{0};
  base::String thread_name_;
  base::Function<void()> run_functor_;
};

// core threading primitives
BASE_EXPORT u32 GetCurrentThreadIndex();
BASE_EXPORT Thread::Handle GetCurrentThreadHandle();

BASE_EXPORT bool SetThreadName(Thread::Handle, const char* name);
inline bool SetCurrentThreadName(const char* name) {
  return SetThreadName(GetCurrentThreadHandle(), name);
}

BASE_EXPORT void SetThreadPriority(Thread::Handle, Thread::Priority new_priority);
BASE_EXPORT Thread::Priority GetThreadPriority(Thread::Handle);
BASE_EXPORT i32 GetNativeThreadPriority(Thread::Handle);
}  // namespace base
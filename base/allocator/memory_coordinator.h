// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/allocator/memory_literals.h>
#include <base/allocator/memory_stat_tracker.h>

namespace base {
struct DefaultRouter {
  void* Allocate(size_t size) {}
  void Free(void* block) {}
};

// this struct musn't instantiate any complex objects
template <class TRouter>
struct MCInstance {
  void* Allocate(size_t size);
  void Free(void* address);

  auto& tracker() { return tracker_; }

 private:
  MemoryTracker tracker_;
  TRouter router_;
};
using MemoryCoordinator = MCInstance<DefaultRouter>;

// get the global instance
STRONG_INLINE MemoryCoordinator& memory_coordinator();

using OutOfMemoryHandler = void(MemoryCoordinator&, void*);
void SetOutOfMemoryHandler(OutOfMemoryHandler*, void* user_context = nullptr);
void InvokeOutOfMemoryHandler();

// TBD: capture line, file, etc
#define BASE_INVOKE_OOM
}  // namespace base

// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/allocator/memory_error.h>
#include <base/allocator/memory_coordinator.h>

namespace base {
namespace {
void DefaultOOMHandler(void* user_pointer, const char* reason) {
  (void)user_pointer;
  (void)reason;

  DEBUG_TRAP;
}

constinit struct {
  OutOfMemoryHandler* handler{DefaultOOMHandler};
  void* user_context{};
} oom_data;
}  // namespace

namespace detail {
void InvokeOutOfMemoryHandler(const base::SourceLocation&, const char* reason) {
  // TODO: pass the source location through.
  // Redzone memory: a pre-reserved tiny segment for throwing the error.
  oom_data.handler(oom_data.user_context, reason);
}
}  // namespace detail

void SetOutOfMemoryHandler(OutOfMemoryHandler* new_handler, void* user_context) {
  oom_data = {new_handler, user_context};
}

void SetOutOfMemoryContext(void* user_context) {
  oom_data.user_context = user_context;
}
}  // namespace base
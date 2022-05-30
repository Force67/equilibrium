// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// This implementation is larged based on the brilliant gdc talk by the folks at
// bluepoint studios. https://www.youtube.com/watch?v=fcBZEZWGYek

#include <base/memory/memory_literals.h>
#include <base/allocator/memory_coordinator.h>

namespace base {
namespace {
// no constructor desired
constinit MemoryCoordinator MemoryRouter{};

void DefaultOOMHandler(MemoryCoordinator&, void*) {
  DEBUG_TRAP;
}

constinit struct {
  OutOfMemoryHandler* handler{DefaultOOMHandler};
  void* user_context{};
} oom_data;
}  // namespace

MemoryCoordinator& memory_coordinator() {
  return MemoryRouter;
}

void SetOutOfMemoryHandler(OutOfMemoryHandler* new_handler, void* user_context) {
  oom_data = {new_handler, user_context};
}

void InvokeOutOfMemoryHandler() {
  // give redzone memory (a prereserved tiny segment for throwing the error.)
  oom_data.handler(MemoryRouter, oom_data.user_context);
}
}  // namespace base
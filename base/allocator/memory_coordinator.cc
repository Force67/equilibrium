// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// This implementation is larged based on the brilliant gdc talk by the folks at
// bluepoint studios. https://www.youtube.com/watch?v=fcBZEZWGYek

#include <base/allocator/memory_literals.h>
#include <base/allocator/memory_coordinator.h>

namespace base {
namespace {
// no constructor desired
constinit MemoryCoordinator MemoryRouter{};

void DefaultOOMHandler(MemoryCoordinator&, void*) {
  DEBUG_TRAP;
}

void* OOM_user_context{nullptr};
constinit OutOfMemoryHandler* OOMHandler{DefaultOOMHandler};
}  // namespace

MemoryCoordinator& memory_coordinator() {
  return MemoryRouter;
}

void SetOutOfMemoryHandler(OutOfMemoryHandler* new_handler, void* user_context) {
  OOMHandler = new_handler;
}

void InvokeOutOfMemoryHandler() {
  // give redzone memory (a prereserved tiny segment for throwing the error.)
  OOMHandler(MemoryRouter, OOM_user_context);
}
}  // namespace base
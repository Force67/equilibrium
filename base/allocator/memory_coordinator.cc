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

template <class TRouter>
inline void* MCInstance<TRouter>::Allocate(size_t size) {
  tracker_.TrackAllocation(size);
  return router_.Allocate(size);
}
template <class TRouter>
inline void MCInstance<TRouter>::Free(void* address) {
  // TODO: determine the size freed
  tracker_.TrackDeallocation(0);
  router_.Free(address);
}

void SetOutOfMemoryHandler(OutOfMemoryHandler* new_handler, void* user_context) {
  OOMHandler = new_handler;
}

void InvokeOutOfMemoryHandler() {
  // give redzone memory (a prereserved tiny segment for throwing the error.)
  OOMHandler(MemoryRouter, OOM_user_context);
}
}  // namespace base
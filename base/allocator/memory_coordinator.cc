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
}  // namespace

MemoryCoordinator& memory_coordinator() {
  return MemoryRouter;
}
}  // namespace base
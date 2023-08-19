
#include "memory_coordinator.h"

namespace base::detail {
// this is smart, all of our arrays are nulled without having to call the ctor
// since we dont store any complex objects
static constinit u8 TheMemoryTracker[sizeof(MemoryTracker)]{0};

static MemoryCoordinator TheMemoryCoordinator{};
}  // namespace base::detail

namespace base {
MemoryCoordinator& memory_coordinator() {
  return detail::TheMemoryCoordinator;
}

MemoryTracker& memory_tracker() {
  return reinterpret_cast<MemoryTracker&>(detail::TheMemoryTracker);
}
}  // namespace base

#include "memory_coordinator.h"

namespace base::detail {
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
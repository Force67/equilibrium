
#include "memory_coordinator.h"

namespace base::detail {
static MemoryCoordinator TheMemoryCoordinator{};
}  // namespace base::detail

namespace base {
MemoryCoordinator& memory_coordinator() {
  return detail::TheMemoryCoordinator;
}
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "new_memory_coordinator.h"

namespace base {

namespace detail {
// where the MC class lives.
alignas(NewMemoryCoordinator) constinit byte
    new_mc_data[sizeof(NewMemoryCoordinator)]{0};
}  // namespace detail

void NewMemoryCoordinator::Create() {
  new (detail::new_mc_data) NewMemoryCoordinator();
}

void NewMemoryCoordinator::Destroy() {
  new_memory_coordinator().~NewMemoryCoordinator();
}

NewMemoryCoordinator::NewMemoryCoordinator() {

}

NewMemoryCoordinator::~NewMemoryCoordinator() {

}

}  // namespace base
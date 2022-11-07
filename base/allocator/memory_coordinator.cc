// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "new_memory_coordinator.h"

namespace base {

namespace detail {
// where the MC class lives.
alignas(NewMemoryCoordinator) constinit byte
    new_mc_data[sizeof(NewMemoryCoordinator)]{0xFF};
}  // namespace detail

bool NewMemoryCoordinator::Create() {
  // mc was already in use.
  if (detail::new_mc_data[0] != 0xFF) {
    return false;
  }
  // placement new
  new (detail::new_mc_data) NewMemoryCoordinator();
  return true;
}

void NewMemoryCoordinator::Destroy() {
  new_memory_coordinator().~NewMemoryCoordinator();
  // no memset, as we don't know if it depends on the CRT in possible implementations
  for (mem_size i = 0; i < sizeof(detail::new_mc_data); i++) {
    detail::new_mc_data[i] = 0xFF;
  }
}

NewMemoryCoordinator::NewMemoryCoordinator() {}
NewMemoryCoordinator::~NewMemoryCoordinator() {}
}  // namespace base
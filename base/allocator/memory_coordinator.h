// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>            //< for STRONG_INLINE
#include <base/profiling/profiler.h>  //< for BASE_PROFILE_ALLOCATION

#include <base/allocator/allocator_primitives.h>
#include <base/allocator/memory_stat_tracker.h>
#include <base/allocator/new_memory_router.h>

namespace base {

class NewMemoryCoordinator {
 public:
  // static initialization methods for initializing/killing the memory subsystem
  // these are special methods, in which the following may not be done:
  // - allocate memory
  // - declare static variables
  // do *NEVER* call these manually
  static bool Create();
  static void Destroy();
 private:
  // same as above applies also here, as well as to any methods being called from the
  // ctor.
  NewMemoryCoordinator();
  ~NewMemoryCoordinator();

 public:
  STRONG_INLINE MemoryTracker& memory_tracker() { return memory_tracker_; }
  STRONG_INLINE NewMemoryRouter& memory_router() { return memory_router_; }

 private:
  MemoryTracker memory_tracker_;
  NewMemoryRouter memory_router_;
};

// implementation detail, do not touch.
namespace detail {
extern byte new_mc_data[];
}

STRONG_INLINE NewMemoryCoordinator& new_memory_coordinator() noexcept {
  return *reinterpret_cast<NewMemoryCoordinator*>(&base::detail::new_mc_data[0]);
}
}  // namespace base
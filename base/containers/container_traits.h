// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/allocator_primitives.h>

namespace base {
template <typename T>
concept HasContainerTraits = requires(T& t) {
  t.data();
  t.size();
};


// TODO: move
struct DefaultAllocator {
  static void* Allocate(mem_size sz) { return base::Allocate(sz); }
  static void Free(void* former, mem_size former_size) { base::Free(former); }
};
}  // namespace base
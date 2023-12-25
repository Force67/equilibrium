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
  static void* Allocate(mem_size byte_size) {
    return base::allocator_primitives::Allocate(byte_size);
  }
  static void Free(void* block, mem_size former_block_byte_size) {
    base::allocator_primitives::Free(block);
  }
};
}  // namespace base
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <new>

namespace base {
template <typename T>
concept HasContainerTraits = requires(T& t) {
  t.data();
  t.size();
};

// Default allocator for all base containers.
// Goes straight through operator new/delete so it respects any global
// allocator override (e.g. mimalloc-new-delete.h) without needing
// equilibrium's memory coordinator / CRT router machinery.
struct DefaultAllocator {
  static void* Allocate(mem_size byte_size) {
    return ::operator new(byte_size);
  }
  static void Free(void* block, mem_size /*former_block_byte_size*/) {
    ::operator delete(block);
  }
};
}  // namespace base

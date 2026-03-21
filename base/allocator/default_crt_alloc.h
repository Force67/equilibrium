// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdlib>
#include <cstring>
#include <base/arch.h>

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(OS_POSIX)
#include <malloc.h>
#define BASE_POSIX_ALLOC 1
#elif defined(_WIN32) || defined(OS_WIN)
#include <malloc.h>
#define BASE_WIN_ALLOC 1
#endif

namespace base {

// CRT router used by the memory coordinator for tracked allocations.
// NOTE: base containers (Vector, String, etc.) bypass this entirely via
// DefaultAllocator which goes straight through operator new/delete.
class DefaultCRTRouter {
 public:
  void* Allocate(mem_size size) { return ::malloc(size); }

  void* AllocateAligned(mem_size size, mem_size alignment) {
#if defined(BASE_WIN_ALLOC)
    return ::_aligned_malloc(size, alignment);
#elif defined(BASE_POSIX_ALLOC)
    return ::aligned_alloc(alignment, size);
#else
    return ::malloc(size);
#endif
  }

  void* ReAllocate(void* former, mem_size new_size, pointer_diff& diff_out) {
    diff_out = 0;
    return ::realloc(former, new_size);
  }

  void* ReAllocateAligned(void* former_block, mem_size former_size,
                          mem_size new_size, mem_size alignment) {
    (void)former_size;
#if defined(BASE_WIN_ALLOC)
    return ::_aligned_realloc(former_block, new_size, alignment);
#elif defined(BASE_POSIX_ALLOC)
    if (former_size >= new_size) return former_block;
    void* new_block = ::aligned_alloc(alignment, new_size);
    if (new_block && former_block) {
      memcpy(new_block, former_block, former_size);
      free(former_block);
    }
    return new_block;
#else
    return ::realloc(former_block, new_size);
#endif
  }

  mem_size Free(void* block) {
    ::free(block);
    return 0;
  }

  bool Deallocate(void* block, mem_size size, mem_size alignment) {
    (void)alignment;
    (void)size;
    ::free(block);
    return true;
  }
};
}  // namespace base

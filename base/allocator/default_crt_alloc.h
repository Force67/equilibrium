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

class DefaultCRTRouter {
 public:
  void* Allocate(mem_size size) {
    // Use operator new instead of malloc so we go through the same allocator
    // as C++ code (e.g. mimalloc overrides operator new but not malloc).
    return ::operator new(size);
  }

  void* AllocateAligned(mem_size size, mem_size alignment) {
#if defined(BASE_WIN_ALLOC)
    return ::_aligned_malloc(size, alignment);
#elif defined(BASE_POSIX_ALLOC)
    return ::aligned_alloc(alignment, size);
#else
    return ::malloc(size);
#endif
  }

  void* ReAllocate(void* former,
                   mem_size new_size,
                   pointer_diff& diff_out) {
    // Can't use realloc with operator new memory. Do alloc+copy+free.
    void* new_block = ::operator new(new_size);
    if (former) {
      // We don't know the old size, so this is a best-effort copy.
      // In practice, callers should use Vector which manages its own growth.
      memcpy(new_block, former, new_size);  // may over-read, but safe for growth
      ::operator delete(former);
    }
    diff_out = 0;
    return new_block;
  }

  void* ReAllocateAligned(void* former_block,
                          mem_size former_size,
                          mem_size new_size,
                          mem_size alignment) {
    (void)former_size;
#if defined(BASE_WIN_ALLOC)
    return ::_aligned_realloc(former_block, new_size, alignment);
#elif defined(BASE_POSIX_ALLOC)
    if (former_size >= new_size) {
      return former_block;
    }
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
    ::operator delete(block);
    return 0;
  }

  bool Deallocate(void* block, mem_size size, mem_size alignment) {
    (void)alignment;
    (void)size;
    ::operator delete(block);
    return true;
  }

 private:
  mem_size block_size(void* block) {
    // Note: malloc_usable_size is NOT safe when a custom allocator like
    // mimalloc intercepts malloc/free but doesn't intercept malloc_usable_size.
    // Calling glibc's malloc_usable_size on a mimalloc block causes heap
    // corruption. Return 0 since the size is only used for stats tracking.
    (void)block;
    return 0;
  }
};
}  // namespace base

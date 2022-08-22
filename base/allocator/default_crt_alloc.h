// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// if neither EQAlloc or any other custom allocators are used we default to this one.
#pragma once

#include <malloc.h>
#include <base/arch.h>

namespace base {

class DefaultCRTRouter {
 public:
     // TODO(V): Consider _aligned_malloc
  void* Allocate(mem_size size) { return ::malloc(size); }

  void* ReAllocate(void* former,
                   mem_size new_size,
                   pointer_diff& diff_out /*could be negative*/) {
    diff_out = new_size - block_size(former);
    return ::realloc(former, new_size);
  }

  // deallocate and report the amount free'd
  mem_size Free(void* block) {
    const mem_size former_block_size{block_size(block)};
    ::free(block);
    return former_block_size;
  }

 private:
  mem_size block_size(void* block) {
    return 0;


    if (!block)
      __debugbreak();

    //DCHECK(block);

#if defined(OS_WIN)
    const mem_size block_size{::_msize(block)};
#elif defined(OS_POSIX)
    const mem_size block_size{::msize(block)};
#else
#error Need to implement
#endif
    return block_size;
  }
};
}  // namespace base
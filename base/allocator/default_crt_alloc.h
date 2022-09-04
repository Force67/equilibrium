// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// if neither EQAlloc or any other custom allocators are used we default to this one.
#pragma once

#include <malloc.h>

#include <base/arch.h>
//#include <base/allocator/allocator_primitives.h>  //< for MemoryBlock struct

namespace base {

class DefaultCRTRouter {
 public:
  void* Allocate(mem_size size) { return ::malloc(size); }
  void* AllocateAligned(mem_size size, mem_size alignment) {
    // If alignment is not a power of 2 or size is zero, this function invokes the
    // invalid parameter handler, as described in Parameter Validation. If execution
    // is allowed to continue, this function returns NULL and sets errno to EINVAL.
    return ::_aligned_malloc(size, alignment);
  }

  void* ReAllocate(void* former,
                   mem_size new_size,
                   pointer_diff& diff_out /*may be negative*/) {
    diff_out = new_size - block_size(former);
    return ::realloc(former, new_size);
  }
  void* ReAllocateAligned(void* former_block,
                          mem_size former_size,
                          mem_size new_size,
                          mem_size alignment) {
    (void)former_size;  // this implementation doesn't benifit from it.
    return ::_aligned_realloc(former_block, new_size, alignment);
  }

  // deallocate and report the amount free'd
  mem_size Free(void* block) {
    const mem_size former_block_size{block_size(block)};
    ::free(block);
    return former_block_size;
  }
  bool Deallocate(void* block, mem_size size, mem_size alignment) {
    // cant take advantage of alignment or size
    (void)alignment;
    (void)size;

    ::_aligned_free(block);
    return true;  // no way for us to know, pretend it worked.
  }

 private:
  mem_size block_size(void* block) {
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
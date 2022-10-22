// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
#pragma once

#include <base/meta/source_location.h>

namespace base::allocator_primitives {
// like c malloc
void* Allocate(mem_size size);

// like c realloc
void* ReAllocate(void* former, mem_size new_size);

// like c free
void Free(void* block);

// the v2 api replaces the default allocation scheme with a more granular model,
// proposed here: https://www.foonathan.net/2022/08/malloc-interface/#content
namespace v2 {
// requires explicit user opt in by casting to this value.
enum class AlignmentValue : mem_size {};

// must be managed by the user api.
struct MemoryBlock {
  void* pointer;
  mem_size size;
};

MemoryBlock Allocate(mem_size size, AlignmentValue alignment);
bool Deallocate(MemoryBlock block, AlignmentValue alignment);

// replacement for ReAllocate.
MemoryBlock TryExpand(MemoryBlock block,
                      mem_size new_size,
                      AlignmentValue alignment);

}  // namespace v2

// implements "utopia alloc", a much more granular system that places *much* more
// control in the hands of the user at the cost of increased complexity. If you only
// want to use the crt allocator, you tend to not see a huge advantage from this.
namespace v3 {
enum class AllocationFlags : u32 {
  None = 0,
  ZeroBlock = 1 << 0,
  MayMove = 1 << 1,
  AlignSize = 1 << 2,
  SmallAlign = 1 << 3,
  NoPointers = 1 << 4,
  FreeOnFailure = 1 << 5,
  SizeIsAHint = 1 << 6,
  Compact = 1 << 7,

  FreeOnSameThreadRequired = 1 << 8,
  FreeOnOtherThreadRequired = 1 << 9,
  OnlyUsedOnCalleeThread = 1 << 10,
  SharedThreadsMostlyWriter = 1 << 11,
  SharedThreads = 1 << 12,

  // cpu cache flags
  AligntoCacheline = 1 << 13,

  // cause we are dealing with bitflags
  Last,
  Max = Last - 1,
};

enum class AllocationHints : u32 {
  None,
  AssumeStack = 1 << 0,
  AssumeShortLifetime = 1 << 1,
  AssumeLongLifetime = 1 << 2,
  AssumeNoFree = 1 << 3,

  // cause we are dealing with bitflags
  Last,
  Max = Last - 1,
};

enum class AlignAndSkew : mem_size {};

struct MemoryBlockV3 {
  void* pointer;
  mem_size size;

  union {
    u64 tracking_flags;
    struct {
      AllocationFlags flags;
      AllocationHints hints;
    } tracking;
  };
};

MemoryBlockV3 Allocate(mem_size size,
                       AlignAndSkew align_and_skew,
                       AllocationFlags flags = AllocationFlags::None,
                       AllocationHints hints = AllocationHints::None);
bool Deallocate(MemoryBlockV3 info, AlignAndSkew align_and_skew);

}  // namespace v3

// use MAKE_SOURCE_LOC and you can track the origin
void* AllocateTracked(mem_size size, const base::SourceLocation&);
void FreeTracked(void* block, const base::SourceLocation&);
}  // namespace base::allocator_primitives

// use these instead of raw new/delete
#if defined(BASE_HARDCORE_MEMORY_TRACKING)
#endif

#if defined(CONFIG_DEBUG)
#define EQ_NEW ::new;
#define EQ_DELETE ::delete;
#else
#define EQ_NEW ::new;
#define EQ_DELETE ::delete;
#endif

#define EQ_MALLOC(x) ::base::allocator_primitives::Allocate(x)
#define EQ_FREE(x) ::base::allocator_primitives::Free(x)
#define EQ_REALLOC(former, new_size) \
  base::allocator_primitives::ReAllocate(former, new_size)
#define EQ_VALLOC(x) __debugbreak()
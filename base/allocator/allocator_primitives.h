// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
#pragma once

#include <base/source_location.h>

namespace base {
// drop in replacements for standard c malloc
void* Allocate(size_t size);
// Changes the size of the memory block pointed to by ptr.
void* ReAllocate(void* former, size_t new_size);

void Free(void* block);

// use MAKE_SOURCE_LOC and you can track the origin
void* AllocateTracked(size_t size, const base::SourceLocation&);
void FreeTracked(void* block, const base::SourceLocation&);
}  // namespace base

#include <base/allocator/allocator_symbol_override.in>

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

#define EQ_MALLOC(x) base::Allocate(x)
#define EQ_FREE(x) base::Free(x)
#define EQ_REALLOC(x) __debugbreak()
#define EQ_VALLOC(x) __debugbreak()
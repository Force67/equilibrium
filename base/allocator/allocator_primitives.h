// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
// Alternative name: memory_primitives.
#pragma once

namespace base {
// these free functions are intended as a dropin replacement
// for malloc/free
void* Allocate(size_t size);
void Free(void* block);

void MarkAllocLocation(const char* name) {}
}  // namespace base

#include <base/allocator/allocator_symbol_override.in>

#define BPE_NEW ::new;
#define BPE_DELETE ::delete;

#define BPE_MALLOC(x) base::Allocate(x)
#define BPE_FREE(x) base::Free(x)
#define BPE_REALLOC(x) __debugbreak()
#define BPE_VALLOC(x) __debugbreak()
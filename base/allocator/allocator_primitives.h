// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Memory primitives.
// Alternative name: memory_primitives.
#pragma once

namespace base {
void* XAlloc(size_t size);
void XFree(void* block);
}  // namespace base

#include <base/allocator/allocator_symbol_override.in>

#define BPE_NEW ::new;
#define BPE_DELETE ::delete;

#define BPE_MALLOC(x) base::XAlloc(x)
#define BPE_FREE(x) base::XFree(x)
#define BPE_REALLOC(x) __debugbreak()
#define BPE_VALLOC(x) __debugbreak()
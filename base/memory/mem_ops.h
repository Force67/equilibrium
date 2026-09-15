// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// The raw memory block operations, without <string.h>.
//
// These four are not really library functions: a compiler emits calls to
// memcpy and memset on its own for struct assignment and zero-init, so the
// symbols are part of the freestanding contract and cannot be refused. What
// can be refused is the header, and with it the rest of <string.h>.
//
// On GCC and Clang the __builtin_ forms need no declaration at all, constant
// fold, and inline for a known size. MSVC has no __builtin_memcpy, but does
// treat these four as intrinsics once declared, which is what the pragma
// below asks for, so both paths inline and neither includes a CRT header.

#pragma once

#include <base/arch.h>
#include <base/compiler.h>

#if defined(_MSC_VER) && !defined(__clang__)
// Declared exactly as <string.h> declares them, including the calling
// convention, so the redeclaration matches. decltype(sizeof(0)) is size_t by
// definition, which matters because base/arch.h deliberately includes nothing
// and so never defines the name.
extern "C" {
void* __cdecl memcpy(void* destination, const void* source, decltype(sizeof(0)) count);
void* __cdecl memmove(void* destination, const void* source, decltype(sizeof(0)) count);
void* __cdecl memset(void* destination, int value, decltype(sizeof(0)) count);
int __cdecl memcmp(const void* lhs, const void* rhs, decltype(sizeof(0)) count);
}
#pragma intrinsic(memcpy, memset, memcmp)
#endif

namespace base {

// Copy |count| bytes. The ranges must not overlap; use MemMove if they may.
STRONG_INLINE void* MemCopy(void* destination, const void* source,
                            mem_size count) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  return memcpy(destination, source, count);
#else
  return __builtin_memcpy(destination, source, count);
#endif
}

// Copy |count| bytes, correctly even when the ranges overlap.
STRONG_INLINE void* MemMove(void* destination, const void* source,
                            mem_size count) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  return memmove(destination, source, count);
#else
  return __builtin_memmove(destination, source, count);
#endif
}

// Write |value| to |count| bytes. The value is taken modulo 256, as memset
// does, not widened.
STRONG_INLINE void* MemSet(void* destination, int value,
                           mem_size count) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  return memset(destination, value, count);
#else
  return __builtin_memset(destination, value, count);
#endif
}

// Zero |count| bytes. Named apart from MemSet because that is what almost
// every caller wants, and because it cannot be given the argument order wrong.
STRONG_INLINE void* MemZero(void* destination, mem_size count) noexcept {
  return MemSet(destination, 0, count);
}

// Ordering comparison over |count| bytes: negative, zero or positive. Note
// this compares bytes, not objects: padding counts.
STRONG_INLINE int MemCompare(const void* lhs, const void* rhs,
                             mem_size count) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  return memcmp(lhs, rhs, count);
#else
  return __builtin_memcmp(lhs, rhs, count);
#endif
}

// Whether |count| bytes are identical. Says what the call site means, and
// reads better than `== 0`.
STRONG_INLINE bool MemEqual(const void* lhs, const void* rhs,
                            mem_size count) noexcept {
  return MemCompare(lhs, rhs, count) == 0;
}

}  // namespace base

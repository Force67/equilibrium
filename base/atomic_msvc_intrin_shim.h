// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// TEST ONLY. Stands the MSVC intrinsics that base/atomic.h's MSVC backend
// calls up on the GCC/Clang __atomic_* builtins, so that backend can be run
// on a host without MSVC. Nothing in base proper includes this; the only
// consumer is atomic_msvc_backend_test.cc, via BASE_ATOMIC_INTRIN_HEADER.
//
// What this does and does not prove: it exercises the backend's logic --
// which intrinsic each width dispatches to, the argument order of the
// compare-exchange family, the bit-casting between T and its integer
// representation, the negation behind fetch_sub, and the byte scaling behind
// pointer fetch_add. It cannot check the emitted code or the placement of
// real barriers, because the shim's own ordering comes from the builtins.
#pragma once

#define _ReadWriteBarrier() __atomic_signal_fence(__ATOMIC_SEQ_CST)
inline void MemoryBarrier() { __atomic_thread_fence(__ATOMIC_SEQ_CST); }

// __iso_volatile_*: an access of that width with no implied ordering. On the
// real thing single-copy atomicity comes from the hardware for an aligned
// access of these widths, and only the ordering is left to the caller's
// barrier -- so a relaxed atomic is the faithful stand-in. A plain *p would
// read the same value but is a race under the C++ memory model, which
// ThreadSanitizer reports and which would bury a genuine finding in noise.
#define SHIM_ISO(BITS, TYPE)                                                 \
  inline TYPE __iso_volatile_load##BITS(const volatile TYPE* p) {            \
    return __atomic_load_n(const_cast<const TYPE*>(p), __ATOMIC_RELAXED);    \
  }                                                                          \
  inline void __iso_volatile_store##BITS(volatile TYPE* p, TYPE v) {         \
    __atomic_store_n(const_cast<TYPE*>(p), v, __ATOMIC_RELAXED);             \
  }

SHIM_ISO(8, char)
SHIM_ISO(16, short)
SHIM_ISO(32, int)
SHIM_ISO(64, long long)
#undef SHIM_ISO

// _InterlockedCompareExchange*(destination, exchange, comparand) -> value found
template <class T>
static inline T ShimCas(volatile T* p, T desired, T expected) {
  T e = expected;
  __atomic_compare_exchange_n(const_cast<T*>(p), &e, desired, false,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return e;  // the value found, whether or not it matched
}
inline char    _InterlockedCompareExchange8(volatile char* p, char d, char e)          { return ShimCas(p, d, e); }
inline short   _InterlockedCompareExchange16(volatile short* p, short d, short e)      { return ShimCas(p, d, e); }
inline int     _InterlockedCompareExchange(volatile int* p, int d, int e)              { return ShimCas(p, d, e); }
inline long long _InterlockedCompareExchange64(volatile long long* p, long long d, long long e){ return ShimCas(p, d, e); }

#define SHIM_RMW(NAME, OP)                                                     \
  inline char    NAME##8(volatile char* p, char v)       { return __atomic_##OP(const_cast<char*>(p), v, __ATOMIC_SEQ_CST); }    \
  inline short   NAME##16(volatile short* p, short v)    { return __atomic_##OP(const_cast<short*>(p), v, __ATOMIC_SEQ_CST); }   \
  inline long long NAME##64(volatile long long* p, long long v){ return __atomic_##OP(const_cast<long long*>(p), v, __ATOMIC_SEQ_CST); } \
  inline int     NAME(volatile int* p, int v)            { return __atomic_##OP(const_cast<int*>(p), v, __ATOMIC_SEQ_CST); }

SHIM_RMW(_InterlockedExchangeAdd, fetch_add)
SHIM_RMW(_InterlockedAnd, fetch_and)
SHIM_RMW(_InterlockedOr, fetch_or)
SHIM_RMW(_InterlockedXor, fetch_xor)

inline char    _InterlockedExchange8(volatile char* p, char v)        { return __atomic_exchange_n(const_cast<char*>(p), v, __ATOMIC_SEQ_CST); }
inline short   _InterlockedExchange16(volatile short* p, short v)     { return __atomic_exchange_n(const_cast<short*>(p), v, __ATOMIC_SEQ_CST); }
inline int     _InterlockedExchange(volatile int* p, int v)           { return __atomic_exchange_n(const_cast<int*>(p), v, __ATOMIC_SEQ_CST); }
inline long long _InterlockedExchange64(volatile long long* p, long long v) { return __atomic_exchange_n(const_cast<long long*>(p), v, __ATOMIC_SEQ_CST); }

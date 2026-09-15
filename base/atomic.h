// Copyright (C) 2022-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Atomic<T>, over one of two native backends:
//
//   - GCC and Clang (including clang-cl on Windows): the __atomic_* builtins
//   - MSVC's own frontend, which has no __atomic_*: _Interlocked* intrinsics
//
// Define BASE_USE_STD_ATOMIC to route through <atomic> instead. Nothing sets
// it automatically; it is an escape hatch for a compiler neither backend
// covers, and it is the only way any part of base pulls in the STL.

#pragma once

#include <base/arch.h>
#include <base/compiler.h>
#include <base/meta/traits.h>

#if defined(BASE_USE_STD_ATOMIC) && BASE_USE_STD_ATOMIC

#include <atomic>

namespace base {

using memory_order = std::memory_order;

inline constexpr auto memory_order_relaxed = std::memory_order_relaxed;
inline constexpr auto memory_order_consume = std::memory_order_consume;
inline constexpr auto memory_order_acquire = std::memory_order_acquire;
inline constexpr auto memory_order_release = std::memory_order_release;
inline constexpr auto memory_order_acq_rel = std::memory_order_acq_rel;
inline constexpr auto memory_order_seq_cst = std::memory_order_seq_cst;

template <class T>
using Atomic = std::atomic<T>;

}  // namespace base

#else  // native implementation

#if !defined(BASE_ATOMIC_MSVC_INTRINSICS)
#  if defined(_MSC_VER) && !defined(__clang__)
#    define BASE_ATOMIC_MSVC_INTRINSICS 1
#  else
#    define BASE_ATOMIC_MSVC_INTRINSICS 0
#  endif
#endif

#if BASE_ATOMIC_MSVC_INTRINSICS
// Both seams above exist so atomic_msvc_backend_test.cc can drive this backend
// on a host without MSVC, standing the intrinsics up on the builtins. That
// test is the only thing that ever exercises the code below off Windows, so
// the seam earns its keep: without it the backend would ship unexecuted.
#  if defined(BASE_ATOMIC_INTRIN_HEADER)
#    include BASE_ATOMIC_INTRIN_HEADER
#  else
#    include <intrin.h>
#  endif
#endif

namespace base {

#if BASE_ATOMIC_MSVC_INTRINSICS
// MSVC has no __ATOMIC_* macros. The values are the standard ones, which is
// all the MSVC backend needs -- it dispatches on them, it does not forward
// them to a builtin.
enum memory_order : int {
  memory_order_relaxed = 0,
  memory_order_consume = 1,
  memory_order_acquire = 2,
  memory_order_release = 3,
  memory_order_acq_rel = 4,
  memory_order_seq_cst = 5,
};
#else
// Values match __ATOMIC_* so they pass through to the builtins untranslated.
enum memory_order : int {
  memory_order_relaxed = __ATOMIC_RELAXED,
  memory_order_consume = __ATOMIC_CONSUME,
  memory_order_acquire = __ATOMIC_ACQUIRE,
  memory_order_release = __ATOMIC_RELEASE,
  memory_order_acq_rel = __ATOMIC_ACQ_REL,
  memory_order_seq_cst = __ATOMIC_SEQ_CST,
};
#endif

namespace atomic_detail {

// Failure order of a CAS can't be stronger than the success order, and
// release/acq_rel aren't valid failure orders.
constexpr memory_order cas_failure_order(memory_order success) noexcept {
  switch (success) {
    case memory_order_acq_rel:
      return memory_order_acquire;
    case memory_order_release:
      return memory_order_relaxed;
    default:
      return success;
  }
}

template <class T>
inline constexpr bool is_atomic_arithmetic_v =
    is_integral_v<T> && !is_same_v<T, bool>;

// A lock-free atomic has to be naturally aligned, which alignas(T) alone does
// not guarantee for a small struct (an 8-byte T can declare alignof 4). Only
// ever raises the alignment, so it is a no-op for every scalar and pointer.
template <class T>
inline constexpr mem_size storage_align_v =
    (sizeof(T) <= 16 && (sizeof(T) & (sizeof(T) - 1)) == 0 && sizeof(T) > alignof(T))
        ? sizeof(T)
        : alignof(T);

#if !BASE_ATOMIC_MSVC_INTRINSICS

// ── Backend: GCC / Clang __atomic_* builtins ───────────────────────
// The builtins take the order as a plain int and handle any trivially
// copyable T, falling back to libatomic above the lock-free width.

template <class T>
STRONG_INLINE T Load(const T* target, memory_order mo) noexcept {
  T result;
  __atomic_load(target, &result, static_cast<int>(mo));
  return result;
}

template <class T>
STRONG_INLINE void Store(T* target, T desired, memory_order mo) noexcept {
  __atomic_store(target, &desired, static_cast<int>(mo));
}

template <class T>
STRONG_INLINE T Exchange(T* target, T desired, memory_order mo) noexcept {
  T result;
  __atomic_exchange(target, &desired, &result, static_cast<int>(mo));
  return result;
}

template <class T>
STRONG_INLINE bool CompareExchange(T* target, T& expected, T desired, bool weak,
                                   memory_order success,
                                   memory_order failure) noexcept {
  return __atomic_compare_exchange(target, &expected, &desired, weak,
                                   static_cast<int>(success),
                                   static_cast<int>(failure));
}

template <class T>
STRONG_INLINE T FetchAdd(T* target, T value, memory_order mo) noexcept {
  return __atomic_fetch_add(target, value, static_cast<int>(mo));
}
template <class T>
STRONG_INLINE T FetchSub(T* target, T value, memory_order mo) noexcept {
  return __atomic_fetch_sub(target, value, static_cast<int>(mo));
}
template <class T>
STRONG_INLINE T FetchAnd(T* target, T value, memory_order mo) noexcept {
  return __atomic_fetch_and(target, value, static_cast<int>(mo));
}
template <class T>
STRONG_INLINE T FetchOr(T* target, T value, memory_order mo) noexcept {
  return __atomic_fetch_or(target, value, static_cast<int>(mo));
}
template <class T>
STRONG_INLINE T FetchXor(T* target, T value, memory_order mo) noexcept {
  return __atomic_fetch_xor(target, value, static_cast<int>(mo));
}

// Pointer arithmetic is done in bytes; the caller has already scaled by the
// pointee size, as it must for a void* or an incomplete pointee.
template <class T>
STRONG_INLINE T* FetchAddBytes(T** target, long long bytes,
                               memory_order mo) noexcept {
  return __atomic_fetch_add(target, bytes, static_cast<int>(mo));
}

template <class T>
STRONG_INLINE bool IsLockFree(const T* target) noexcept {
  return __atomic_is_lock_free(sizeof(T), target);
}
template <class T>
inline constexpr bool is_always_lock_free_v =
    __atomic_always_lock_free(sizeof(T), 0);

STRONG_INLINE void ThreadFence(memory_order mo) noexcept {
  __atomic_thread_fence(static_cast<int>(mo));
}

#else  // BASE_ATOMIC_MSVC_INTRINSICS

// ── Backend: MSVC _Interlocked* intrinsics ─────────────────────────
//
// Shape of this backend, and why:
//
//   - Every value is bit-cast to the signed integer of its own width and
//     operated on there, because that is the only shape the intrinsics take.
//     Only 1, 2, 4 and 8 byte types are supported; a static_assert in Atomic
//     rejects the rest rather than silently losing atomicity.
//
//   - Read-modify-write always uses the plain (unsuffixed) intrinsic, which
//     is a full barrier on every target. That is stronger than a relaxed or
//     acquire request asks for, never weaker, so it is always correct -- and
//     on x86/x64 it is exactly what the weaker orders would have compiled to
//     anyway, since every lock-prefixed instruction is already seq_cst.
//
//   - Load and store are the only operations where the order actually
//     changes the emitted code, so they are the only ones that branch on it.

#if defined(_M_ARM64) || defined(_M_ARM64EC)
// ARM is weakly ordered: a real barrier is needed. 0x0B is ISH, the inner
// shareable full barrier, matching what the MS STL emits here.
#  define BASE_ATOMIC_FENCE() __dmb(0x0B)
#else
// x86 and x64 are TSO: loads are already acquire and stores already release,
// so only the compiler has to be stopped from reordering across the access.
#  define BASE_ATOMIC_FENCE() _ReadWriteBarrier()
#endif

// MSVC spells the 32- and 64-bit interlocked families in terms of long and
// __int64, which are 32 and 64 bits there. Name the two widths once rather
// than repeating those spellings: on the host the backend test runs on, long
// is 64 bits, and hard-coding it would silently make every 4-byte atomic
// operate on 8 bytes of storage.
#if defined(_MSC_VER) && !defined(__clang__)
using Int32 = long;
using Int64 = __int64;
#else
using Int32 = int;
using Int64 = long long;
#endif

// The integer each width is punned to, matching the intrinsic signatures.
template <mem_size N>
struct Repr;
template <>
struct Repr<1> {
  using type = char;
};
template <>
struct Repr<2> {
  using type = short;
};
template <>
struct Repr<4> {
  using type = Int32;
};
template <>
struct Repr<8> {
  using type = Int64;
};

template <class T>
using repr_t = typename Repr<sizeof(T)>::type;

template <class T>
STRONG_INLINE repr_t<T> ToRepr(T value) noexcept {
  return __builtin_bit_cast(repr_t<T>, value);
}
template <class T>
STRONG_INLINE T FromRepr(repr_t<T> value) noexcept {
  return __builtin_bit_cast(T, value);
}

// Plain volatile access, width-dispatched. __iso_volatile_* is the access
// with no implied barrier, so the ordering below is entirely explicit rather
// than dependent on whether /volatile:ms or /volatile:iso is in effect.
STRONG_INLINE char IsoLoad(const volatile char* p) noexcept {
  return __iso_volatile_load8(p);
}
STRONG_INLINE short IsoLoad(const volatile short* p) noexcept {
  return __iso_volatile_load16(p);
}
STRONG_INLINE Int32 IsoLoad(const volatile Int32* p) noexcept {
  return static_cast<Int32>(
      __iso_volatile_load32(reinterpret_cast<const volatile int*>(p)));
}
STRONG_INLINE Int64 IsoLoad(const volatile Int64* p) noexcept {
  return __iso_volatile_load64(p);
}

STRONG_INLINE void IsoStore(volatile char* p, char v) noexcept {
  __iso_volatile_store8(p, v);
}
STRONG_INLINE void IsoStore(volatile short* p, short v) noexcept {
  __iso_volatile_store16(p, v);
}
STRONG_INLINE void IsoStore(volatile Int32* p, Int32 v) noexcept {
  __iso_volatile_store32(reinterpret_cast<volatile int*>(p),
                         static_cast<int>(v));
}
STRONG_INLINE void IsoStore(volatile Int64* p, Int64 v) noexcept {
  __iso_volatile_store64(p, v);
}

// Compare-exchange, width-dispatched. Argument order is (destination,
// exchange, comparand) and the return is the value found, not a flag.
STRONG_INLINE char Cas(volatile char* p, char desired, char expected) noexcept {
  return _InterlockedCompareExchange8(p, desired, expected);
}
STRONG_INLINE short Cas(volatile short* p, short desired, short expected) noexcept {
  return _InterlockedCompareExchange16(p, desired, expected);
}
STRONG_INLINE Int32 Cas(volatile Int32* p, Int32 desired, Int32 expected) noexcept {
  return _InterlockedCompareExchange(p, desired, expected);
}
STRONG_INLINE Int64 Cas(volatile Int64* p, Int64 desired,
                        Int64 expected) noexcept {
  return _InterlockedCompareExchange64(p, desired, expected);
}

// 32-bit x86 has cmpxchg8b, so _InterlockedCompareExchange64 exists there,
// but the other 64-bit read-modify-writes do not. Synthesise them from the
// CAS so the backend covers x86 without special-casing at every call site.
#if defined(_M_IX86)
template <class Int, class Op>
STRONG_INLINE Int CasLoop(volatile Int* p, Op op) noexcept {
  for (;;) {
    const Int observed = IsoLoad(p);
    if (Cas(p, op(observed), observed) == observed)
      return observed;
  }
}
#endif

STRONG_INLINE char Xchg(volatile char* p, char v) noexcept {
  return _InterlockedExchange8(p, v);
}
STRONG_INLINE short Xchg(volatile short* p, short v) noexcept {
  return _InterlockedExchange16(p, v);
}
STRONG_INLINE Int32 Xchg(volatile Int32* p, Int32 v) noexcept {
  return _InterlockedExchange(p, v);
}
STRONG_INLINE Int64 Xchg(volatile Int64* p, Int64 v) noexcept {
#if defined(_M_IX86)
  return CasLoop(p, [v](Int64) { return v; });
#else
  return _InterlockedExchange64(p, v);
#endif
}

STRONG_INLINE char Xadd(volatile char* p, char v) noexcept {
  return _InterlockedExchangeAdd8(p, v);
}
STRONG_INLINE short Xadd(volatile short* p, short v) noexcept {
  return _InterlockedExchangeAdd16(p, v);
}
STRONG_INLINE Int32 Xadd(volatile Int32* p, Int32 v) noexcept {
  return _InterlockedExchangeAdd(p, v);
}
STRONG_INLINE Int64 Xadd(volatile Int64* p, Int64 v) noexcept {
#if defined(_M_IX86)
  return CasLoop(p, [v](Int64 observed) { return observed + v; });
#else
  return _InterlockedExchangeAdd64(p, v);
#endif
}

STRONG_INLINE char And(volatile char* p, char v) noexcept {
  return _InterlockedAnd8(p, v);
}
STRONG_INLINE short And(volatile short* p, short v) noexcept {
  return _InterlockedAnd16(p, v);
}
STRONG_INLINE Int32 And(volatile Int32* p, Int32 v) noexcept {
  return _InterlockedAnd(p, v);
}
STRONG_INLINE Int64 And(volatile Int64* p, Int64 v) noexcept {
#if defined(_M_IX86)
  return CasLoop(p, [v](Int64 observed) { return observed & v; });
#else
  return _InterlockedAnd64(p, v);
#endif
}

STRONG_INLINE char Or(volatile char* p, char v) noexcept {
  return _InterlockedOr8(p, v);
}
STRONG_INLINE short Or(volatile short* p, short v) noexcept {
  return _InterlockedOr16(p, v);
}
STRONG_INLINE Int32 Or(volatile Int32* p, Int32 v) noexcept {
  return _InterlockedOr(p, v);
}
STRONG_INLINE Int64 Or(volatile Int64* p, Int64 v) noexcept {
#if defined(_M_IX86)
  return CasLoop(p, [v](Int64 observed) { return observed | v; });
#else
  return _InterlockedOr64(p, v);
#endif
}

STRONG_INLINE char Xor(volatile char* p, char v) noexcept {
  return _InterlockedXor8(p, v);
}
STRONG_INLINE short Xor(volatile short* p, short v) noexcept {
  return _InterlockedXor16(p, v);
}
STRONG_INLINE Int32 Xor(volatile Int32* p, Int32 v) noexcept {
  return _InterlockedXor(p, v);
}
STRONG_INLINE Int64 Xor(volatile Int64* p, Int64 v) noexcept {
#if defined(_M_IX86)
  return CasLoop(p, [v](Int64 observed) { return observed ^ v; });
#else
  return _InterlockedXor64(p, v);
#endif
}

template <class T>
STRONG_INLINE volatile repr_t<T>* AsRepr(T* target) noexcept {
  return reinterpret_cast<volatile repr_t<T>*>(target);
}
template <class T>
STRONG_INLINE const volatile repr_t<T>* AsRepr(const T* target) noexcept {
  return reinterpret_cast<const volatile repr_t<T>*>(target);
}

template <class T>
STRONG_INLINE T Load(const T* target, memory_order mo) noexcept {
  const repr_t<T> value = IsoLoad(AsRepr(target));
  // Acquire and stronger must not let later accesses float above the load.
  if (mo != memory_order_relaxed)
    BASE_ATOMIC_FENCE();
  return FromRepr<T>(value);
}

template <class T>
STRONG_INLINE void Store(T* target, T desired, memory_order mo) noexcept {
  const repr_t<T> value = ToRepr(desired);
  if (mo == memory_order_seq_cst) {
    // A seq_cst store needs a full fence after it as well, which on x86 and
    // x64 is exactly what the implicit lock prefix on xchg provides.
    (void)Xchg(AsRepr(target), value);
    return;
  }
  // Release and weaker: keep earlier accesses from sinking below the store.
  if (mo != memory_order_relaxed)
    BASE_ATOMIC_FENCE();
  IsoStore(AsRepr(target), value);
}

template <class T>
STRONG_INLINE T Exchange(T* target, T desired, memory_order) noexcept {
  return FromRepr<T>(Xchg(AsRepr(target), ToRepr(desired)));
}

template <class T>
STRONG_INLINE bool CompareExchange(T* target, T& expected, T desired,
                                   bool /*weak*/, memory_order,
                                   memory_order) noexcept {
  // There is no load-linked/store-conditional form here, so weak and strong
  // are the same instruction; weak is allowed to not fail spuriously.
  const repr_t<T> wanted = ToRepr(expected);
  const repr_t<T> found = Cas(AsRepr(target), ToRepr(desired), wanted);
  if (found == wanted)
    return true;
  // On failure the standard requires expected to be updated to what was read.
  expected = FromRepr<T>(found);
  return false;
}

template <class T>
STRONG_INLINE T FetchAdd(T* target, T value, memory_order) noexcept {
  return FromRepr<T>(Xadd(AsRepr(target), ToRepr(value)));
}
template <class T>
STRONG_INLINE T FetchSub(T* target, T value, memory_order) noexcept {
  // Negate in the representation type: for an unsigned T the wraparound is
  // exactly the two's complement subtraction xadd performs.
  return FromRepr<T>(
      Xadd(AsRepr(target), static_cast<repr_t<T>>(0 - ToRepr(value))));
}
template <class T>
STRONG_INLINE T FetchAnd(T* target, T value, memory_order) noexcept {
  return FromRepr<T>(And(AsRepr(target), ToRepr(value)));
}
template <class T>
STRONG_INLINE T FetchOr(T* target, T value, memory_order) noexcept {
  return FromRepr<T>(Or(AsRepr(target), ToRepr(value)));
}
template <class T>
STRONG_INLINE T FetchXor(T* target, T value, memory_order) noexcept {
  return FromRepr<T>(Xor(AsRepr(target), ToRepr(value)));
}

template <class T>
STRONG_INLINE T* FetchAddBytes(T** target, long long bytes,
                               memory_order) noexcept {
  using Int = repr_t<T*>;
  return reinterpret_cast<T*>(static_cast<pointer_size>(
      Xadd(reinterpret_cast<volatile Int*>(target), static_cast<Int>(bytes))));
}

template <class T>
STRONG_INLINE bool IsLockFree(const T*) noexcept {
  return sizeof(T) <= 8 && (sizeof(T) & (sizeof(T) - 1)) == 0;
}
template <class T>
inline constexpr bool is_always_lock_free_v =
    sizeof(T) <= 8 && (sizeof(T) & (sizeof(T) - 1)) == 0;

STRONG_INLINE void ThreadFence(memory_order mo) noexcept {
  if (mo == memory_order_relaxed)
    return;
  if (mo == memory_order_seq_cst) {
    // Only seq_cst needs to order a prior store against a later load, which
    // on x86/x64 takes a real serialising instruction rather than a barrier.
    ::MemoryBarrier();
    return;
  }
  BASE_ATOMIC_FENCE();
}

#endif  // BASE_ATOMIC_MSVC_INTRINSICS

}  // namespace atomic_detail

// Primary template. On the builtin backend T may be anything trivially
// copyable, up to the platform's lock-free width (16 B on AMD64 and on LSE
// AArch64); wider T falls off the lock-free path and calls into libatomic.
// The MSVC backend has no such fallback, so there it is 1, 2, 4 or 8 bytes.
template <class T>
class Atomic {
#if BASE_ATOMIC_MSVC_INTRINSICS
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
                    sizeof(T) == 8,
                "base::Atomic on MSVC covers 1, 2, 4 and 8 byte types; a "
                "wider one has no lock-free intrinsic to lower to");
#endif

 public:
  using value_type = T;

  constexpr Atomic() noexcept : value_{} {}
  constexpr Atomic(T desired) noexcept : value_(desired) {}

  Atomic(const Atomic&) = delete;
  Atomic& operator=(const Atomic&) = delete;
  Atomic(Atomic&&) = delete;
  Atomic& operator=(Atomic&&) = delete;

  T operator=(T desired) noexcept {
    store(desired);
    return desired;
  }

  operator T() const noexcept { return load(); }

  bool is_lock_free() const noexcept {
    return atomic_detail::IsLockFree(&value_);
  }
  static constexpr bool is_always_lock_free =
      atomic_detail::is_always_lock_free_v<T>;

  T load(memory_order mo = memory_order_seq_cst) const noexcept {
    return atomic_detail::Load(&value_, mo);
  }

  void store(T desired, memory_order mo = memory_order_seq_cst) noexcept {
    atomic_detail::Store(&value_, desired, mo);
  }

  T exchange(T desired, memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::Exchange(&value_, desired, mo);
  }

  bool compare_exchange_weak(T& expected, T desired,
                             memory_order success,
                             memory_order failure) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/true, success, failure);
  }
  bool compare_exchange_weak(T& expected, T desired,
                             memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/true, mo,
                                          atomic_detail::cas_failure_order(mo));
  }

  bool compare_exchange_strong(T& expected, T desired,
                               memory_order success,
                               memory_order failure) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/false, success, failure);
  }
  bool compare_exchange_strong(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/false, mo,
                                          atomic_detail::cas_failure_order(mo));
  }

  T fetch_add(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return atomic_detail::FetchAdd(&value_, v, mo);
  }
  T fetch_sub(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return atomic_detail::FetchSub(&value_, v, mo);
  }
  T fetch_and(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return atomic_detail::FetchAnd(&value_, v, mo);
  }
  T fetch_or(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return atomic_detail::FetchOr(&value_, v, mo);
  }
  T fetch_xor(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return atomic_detail::FetchXor(&value_, v, mo);
  }

  T operator++() noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_add(1) + 1;
  }
  T operator++(int) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_add(1);
  }
  T operator--() noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_sub(1) - 1;
  }
  T operator--(int) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_sub(1);
  }
  T operator+=(T v) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_add(v) + v;
  }
  T operator-=(T v) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_sub(v) - v;
  }
  T operator&=(T v) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_and(v) & v;
  }
  T operator|=(T v) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_or(v) | v;
  }
  T operator^=(T v) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return fetch_xor(v) ^ v;
  }

 private:
  alignas(atomic_detail::storage_align_v<T>) T value_;
};

// Pointer specialization. fetch_add/sub scale by sizeof(T), as pointer
// arithmetic does; the backends work in bytes.
template <class T>
class Atomic<T*> {
 public:
  using value_type = T*;
  using difference_type = long long;

  constexpr Atomic() noexcept : value_(nullptr) {}
  constexpr Atomic(T* desired) noexcept : value_(desired) {}

  Atomic(const Atomic&) = delete;
  Atomic& operator=(const Atomic&) = delete;

  T* operator=(T* desired) noexcept {
    store(desired);
    return desired;
  }

  operator T*() const noexcept { return load(); }

  bool is_lock_free() const noexcept {
    return atomic_detail::IsLockFree(&value_);
  }
  static constexpr bool is_always_lock_free =
      atomic_detail::is_always_lock_free_v<T*>;

  T* load(memory_order mo = memory_order_seq_cst) const noexcept {
    return atomic_detail::Load(&value_, mo);
  }

  void store(T* desired, memory_order mo = memory_order_seq_cst) noexcept {
    atomic_detail::Store(&value_, desired, mo);
  }

  T* exchange(T* desired, memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::Exchange(&value_, desired, mo);
  }

  bool compare_exchange_weak(T*& expected, T* desired,
                             memory_order success,
                             memory_order failure) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/true, success, failure);
  }
  bool compare_exchange_weak(T*& expected, T* desired,
                             memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/true, mo,
                                          atomic_detail::cas_failure_order(mo));
  }

  bool compare_exchange_strong(T*& expected, T* desired,
                               memory_order success,
                               memory_order failure) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/false, success, failure);
  }
  bool compare_exchange_strong(T*& expected, T* desired,
                               memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::CompareExchange(&value_, expected, desired,
                                          /*weak=*/false, mo,
                                          atomic_detail::cas_failure_order(mo));
  }

  T* fetch_add(difference_type n,
               memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::FetchAddBytes(
        &value_, n * static_cast<difference_type>(sizeof(T)), mo);
  }
  T* fetch_sub(difference_type n,
               memory_order mo = memory_order_seq_cst) noexcept {
    return atomic_detail::FetchAddBytes(
        &value_, -n * static_cast<difference_type>(sizeof(T)), mo);
  }

 private:
  alignas(atomic_detail::storage_align_v<T*>) T* value_;
};

}  // namespace base

#endif  // BASE_USE_STD_ATOMIC

namespace base {

inline void atomic_thread_fence(memory_order mo) noexcept {
#if defined(BASE_USE_STD_ATOMIC) && BASE_USE_STD_ATOMIC
  std::atomic_thread_fence(mo);
#else
  atomic_detail::ThreadFence(mo);
#endif
}

}  // namespace base

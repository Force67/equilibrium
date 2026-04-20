// Copyright (C) 2022-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Atomic<T> built on GCC/Clang __atomic_* builtins. MSVC auto-falls
// back to std::atomic; define BASE_USE_STD_ATOMIC to force that elsewhere.

#pragma once

#include <base/arch.h>
#include <base/meta/traits.h>

#if !defined(BASE_USE_STD_ATOMIC)
#  if defined(_MSC_VER) && !defined(__clang__)
#    define BASE_USE_STD_ATOMIC 1
#  endif
#endif

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

namespace base {

// Values match __ATOMIC_* so they pass through to the builtins untranslated.
enum memory_order : int {
  memory_order_relaxed = __ATOMIC_RELAXED,
  memory_order_consume = __ATOMIC_CONSUME,
  memory_order_acquire = __ATOMIC_ACQUIRE,
  memory_order_release = __ATOMIC_RELEASE,
  memory_order_acq_rel = __ATOMIC_ACQ_REL,
  memory_order_seq_cst = __ATOMIC_SEQ_CST,
};

namespace atomic_detail {

// Failure order of a CAS can't be stronger than the success order, and
// release/acq_rel aren't valid failure orders.
constexpr int cas_failure_order(memory_order success) noexcept {
  switch (success) {
    case memory_order_acq_rel:
      return __ATOMIC_ACQUIRE;
    case memory_order_release:
      return __ATOMIC_RELAXED;
    default:
      return static_cast<int>(success);
  }
}

template <class T>
inline constexpr bool is_atomic_arithmetic_v =
    is_integral_v<T> && !is_same_v<T, bool>;

}  // namespace atomic_detail

// Primary template. Supports T up to the platform's lock-free width (16 B
// on AMD64 + LSE AArch64). Wider T falls off the lock-free path and calls
// into libatomic.
template <class T>
class Atomic {
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
    return __atomic_is_lock_free(sizeof(T), &value_);
  }
  static constexpr bool is_always_lock_free =
      __atomic_always_lock_free(sizeof(T), 0);

  T load(memory_order mo = memory_order_seq_cst) const noexcept {
    T result;
    __atomic_load(&value_, &result, static_cast<int>(mo));
    return result;
  }

  void store(T desired, memory_order mo = memory_order_seq_cst) noexcept {
    __atomic_store(&value_, &desired, static_cast<int>(mo));
  }

  T exchange(T desired, memory_order mo = memory_order_seq_cst) noexcept {
    T result;
    __atomic_exchange(&value_, &desired, &result, static_cast<int>(mo));
    return result;
  }

  bool compare_exchange_weak(T& expected, T desired,
                             memory_order success,
                             memory_order failure) noexcept {
    return __atomic_compare_exchange(&value_, &expected, &desired,
                                     /*weak=*/true,
                                     static_cast<int>(success),
                                     static_cast<int>(failure));
  }
  bool compare_exchange_weak(T& expected, T desired,
                             memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange(&value_, &expected, &desired,
                                     /*weak=*/true,
                                     static_cast<int>(mo),
                                     atomic_detail::cas_failure_order(mo));
  }

  bool compare_exchange_strong(T& expected, T desired,
                               memory_order success,
                               memory_order failure) noexcept {
    return __atomic_compare_exchange(&value_, &expected, &desired,
                                     /*weak=*/false,
                                     static_cast<int>(success),
                                     static_cast<int>(failure));
  }
  bool compare_exchange_strong(T& expected, T desired,
                               memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange(&value_, &expected, &desired,
                                     /*weak=*/false,
                                     static_cast<int>(mo),
                                     atomic_detail::cas_failure_order(mo));
  }

  T fetch_add(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return __atomic_fetch_add(&value_, v, static_cast<int>(mo));
  }
  T fetch_sub(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return __atomic_fetch_sub(&value_, v, static_cast<int>(mo));
  }
  T fetch_and(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return __atomic_fetch_and(&value_, v, static_cast<int>(mo));
  }
  T fetch_or(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return __atomic_fetch_or(&value_, v, static_cast<int>(mo));
  }
  T fetch_xor(T v, memory_order mo = memory_order_seq_cst) noexcept
    requires(atomic_detail::is_atomic_arithmetic_v<T>)
  {
    return __atomic_fetch_xor(&value_, v, static_cast<int>(mo));
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
  alignas(T) T value_;
};

// Pointer specialization. __atomic_fetch_add on T* scales by sizeof(T).
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
    return __atomic_is_lock_free(sizeof(T*), &value_);
  }
  static constexpr bool is_always_lock_free =
      __atomic_always_lock_free(sizeof(T*), 0);

  T* load(memory_order mo = memory_order_seq_cst) const noexcept {
    return static_cast<T*>(
        __atomic_load_n(reinterpret_cast<void* const*>(&value_),
                        static_cast<int>(mo)));
  }

  void store(T* desired, memory_order mo = memory_order_seq_cst) noexcept {
    __atomic_store_n(&value_, desired, static_cast<int>(mo));
  }

  T* exchange(T* desired, memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_exchange_n(&value_, desired, static_cast<int>(mo));
  }

  bool compare_exchange_weak(T*& expected, T* desired,
                             memory_order success,
                             memory_order failure) noexcept {
    return __atomic_compare_exchange_n(&value_, &expected, desired,
                                       /*weak=*/true,
                                       static_cast<int>(success),
                                       static_cast<int>(failure));
  }
  bool compare_exchange_weak(T*& expected, T* desired,
                             memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange_n(&value_, &expected, desired,
                                       /*weak=*/true, static_cast<int>(mo),
                                       atomic_detail::cas_failure_order(mo));
  }

  bool compare_exchange_strong(T*& expected, T* desired,
                               memory_order success,
                               memory_order failure) noexcept {
    return __atomic_compare_exchange_n(&value_, &expected, desired,
                                       /*weak=*/false,
                                       static_cast<int>(success),
                                       static_cast<int>(failure));
  }
  bool compare_exchange_strong(T*& expected, T* desired,
                               memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_compare_exchange_n(&value_, &expected, desired,
                                       /*weak=*/false, static_cast<int>(mo),
                                       atomic_detail::cas_failure_order(mo));
  }

  T* fetch_add(difference_type n,
               memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_fetch_add(&value_, n * static_cast<difference_type>(sizeof(T)),
                              static_cast<int>(mo));
  }
  T* fetch_sub(difference_type n,
               memory_order mo = memory_order_seq_cst) noexcept {
    return __atomic_fetch_sub(&value_, n * static_cast<difference_type>(sizeof(T)),
                              static_cast<int>(mo));
  }

 private:
  T* value_;
};

}  // namespace base

#endif  // BASE_USE_STD_ATOMIC

namespace base {

inline void atomic_thread_fence(memory_order mo) noexcept {
#if defined(BASE_USE_STD_ATOMIC) && BASE_USE_STD_ATOMIC
  std::atomic_thread_fence(mo);
#else
  __atomic_thread_fence(static_cast<int>(mo));
#endif
}

}  // namespace base

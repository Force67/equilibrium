// Copyright (C) 2024-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Hash<T> — default hash functor for container keys.
//
// Design notes:
//   - For integer / pointer / enum keys we use a finalizer derived from
//     SplitMix64 (the same mixer used by xoshiro to scramble a seed).
//     It's branch-free, produces full 64-bit avalanche, and compiles down
//     to three multiplies + two xors.
//   - For raw byte sequences callers can use base::HashBytes() which
//     delegates to FNV-1a.
//   - Users can specialize base::Hash<MyType> to plug in their own hash.
//
// This replaces usage of std::hash inside base containers.

#pragma once

#include <base/arch.h>
#include <base/hashing/fnv1a.h>
#include <base/meta/traits.h>

namespace base {

namespace detail {

// SplitMix64 finalizer. Good avalanche, three imuls + two xors.
constexpr u64 Mix64(u64 x) {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

}  // namespace detail

// Primary template handles integer and enum types. Pointers have their own
// partial specialization below. Anything else must provide an explicit
// specialization.
template <class T>
struct Hash {
  static_assert(base::is_integral_v<T> || base::is_enum_v<T>,
                "base::Hash<T> has no default for this type. "
                "Provide a specialization, or pass a custom hasher.");

  constexpr mem_size operator()(T key) const noexcept {
    if constexpr (base::is_enum_v<T>) {
      using U = base::underlying_type_t<T>;
      return static_cast<mem_size>(
          detail::Mix64(static_cast<u64>(static_cast<U>(key))));
    } else {
      return static_cast<mem_size>(
          detail::Mix64(static_cast<u64>(static_cast<unsigned long long>(key))));
    }
  }
};

template <class T>
struct Hash<T*> {
  constexpr mem_size operator()(T* p) const noexcept {
    return static_cast<mem_size>(
        detail::Mix64(static_cast<u64>(reinterpret_cast<mem_size>(p))));
  }
};

// Raw-bytes convenience — FNV-1a over a buffer.
inline u64 HashBytes(const void* data, mem_size len) {
  return base::fnv1a(static_cast<const u8*>(data), len);
}

}  // namespace base

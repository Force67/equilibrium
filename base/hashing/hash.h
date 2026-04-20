// Copyright (C) 2024-2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Hash<T>: default hash functor for container keys.
// Integers/enums/pointers go through a SplitMix64 finalizer. Byte blobs
// use base::HashBytes (FNV-1a). Specialize base::Hash<T> for other types.

#pragma once

#include <base/arch.h>
#include <base/hashing/fnv1a.h>
#include <base/meta/traits.h>

namespace base {

namespace detail {

// SplitMix64. The constant add breaks the mixer's fixed point at 0.
constexpr u64 Mix64(u64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

}  // namespace detail

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

inline u64 HashBytes(const void* data, mem_size len) {
  return base::fnv1a(static_cast<const u8*>(data), len);
}

}  // namespace base

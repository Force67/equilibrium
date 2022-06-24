// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {
template <class T>
inline constexpr bool IsTrivial = __is_trivially_constructible(T) &&
                                   __is_trivially_copyable(T);

template <typename>
inline constexpr bool IsArray =
    false;  // determine whether type argument is an array

template <typename T, mem_size TSize>
inline constexpr bool IsArray<T[TSize]> = true;

template <typename T>
inline constexpr bool IsArray<T[]> = true;

template <typename, typename>
inline constexpr bool ISSame =
    false;  // determine whether arguments are the same type
template <typename T>
inline constexpr bool ISSame<T, T> = true;

template <class _Ty>
struct remove_extent { // remove array extent
    using type = _Ty;
};
template <class _Ty, size_t _Ix>
struct remove_extent<_Ty[_Ix]> {
    using type = _Ty;
};
template <class _Ty>
struct remove_extent<_Ty[]> {
    using type = _Ty;
};
template <class _Ty>
using remove_extent_t = typename remove_extent<_Ty>::type;

template <typename T>
inline void DestructRange(T first, T last) {
  if constexpr (!IsTrivial<T>) {
    for (; first != last; ++first)
      (*first).~T();
  }
}

}  // namespace base
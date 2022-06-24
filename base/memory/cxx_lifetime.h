// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {
template <class T>
inline constexpr bool IsTrivial = __is_trivially_constructible(T) &&
                                   __is_trivially_copyable(T);

template <typename T>
inline void DestructRange(T first, T last) {
  if constexpr (!IsTrivial<T>) {
    for (; first != last; ++first)
      (*first).~T();
  }
}

}  // namespace base
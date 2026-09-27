// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <initializer_list>

namespace base {
template <typename T>
inline T Min(T a, T b) {
  return (a < b) ? a : b;
}

template <typename T>
inline T Max(T a, T b) {
  return (a > b) ? a : b;
}

template <typename T>
inline T Min(const T* values, int count) {
  T min_val = values[0];
  for (int i = 1; i < count; i++) {
    min_val = (values[i] < min_val) ? values[i] : min_val;
  }
  return min_val;
}

template <typename T>
inline T Max(const T* values, int count) {
  T max_val = values[0];
  for (int i = 1; i < count; i++) {
    max_val = (values[i] > max_val) ? values[i] : max_val;
  }
  return max_val;
}

// The smallest / largest of a non-empty list: base::Min({a, b, c}).
template <typename T>
inline T Min(std::initializer_list<T> values) {
  const T* it = values.begin();
  T result = *it;
  for (++it; it != values.end(); ++it)
    result = (*it < result) ? *it : result;
  return result;
}

template <typename T>
inline T Max(std::initializer_list<T> values) {
  const T* it = values.begin();
  T result = *it;
  for (++it; it != values.end(); ++it)
    result = (*it > result) ? *it : result;
  return result;
}
}  // namespace base
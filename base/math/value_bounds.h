// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

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
}  // namespace base
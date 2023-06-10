// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace ui {
template <typename T = float>
struct Point {
  T x, y;

  Point() : x(0), y(0) {}
  Point(T x, T y) : x(x), y(y) {}
};
}  // namespace ui
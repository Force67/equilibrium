// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace ui {
template <typename T = f32>
struct Point {
  T x, y;

  Point() : x(0), y(0) {}
  Point(T x, T y) : x(x), y(y) {}

  Point operator/(T other_skalar) { return {x / other_skalar, y / other_skalar}; }
  Point operator*(T other_skalar) { return {x * other_skalar, y * other_skalar}; }

  bool operator==(const Point& other) const { return x == other.x && y == other.y; }
  bool operator!=(const Point& other) const { return !(*this == other); }
};

using IPoint = Point<i32>;
using FPoint = Point<f32>;
}  // namespace ui
// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cmath>
#include <base/arch.h>

namespace ui {
template <typename T = f32>
struct Point {
  T x, y;

  // Static Make method
  static Point<T> Make(T x, T y) { return Point<T>(x, y); }

  // Constructors
  Point() : x(0), y(0) {}
  Point(T one) : x(one), y(one) {}
  Point(T x, T y) : x(x), y(y) {}

  // Arithmetic operations with another Point
  Point operator+(const Point& other) const {
    return {x + other.x, y + other.y};
  }
  Point operator-(const Point& other) const {
    return {x - other.x, y - other.y};
  }

  // Scalar multiplication and division
  Point operator*(T scalar) const { return {x * scalar, y * scalar}; }
  Point operator/(T scalar) const { return {x / scalar, y / scalar}; }

  // Negation
  Point operator-() const { return {-x, -y}; }

  // Equality and inequality operators
  bool operator==(const Point& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Point& other) const { return !(*this == other); }

  // Compute distance to another point
  T DistanceTo(const Point& other) const {
    T dx = x - other.x;
    T dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
  }

  // Magnitude (length) of the vector represented by the point
  T magnitude() const { return std::sqrt(x * x + y * y); }

  // Normalize the vector (make it have a length of 1, but keep direction)
  Point normalized() const {
    T mag = magnitude();
    if (mag == 0)
      return {0, 0};
    return {x / mag, y / mag};
  }
};

using IPoint = Point<i32>;
using FPoint = Point<f32>;
}  // namespace ui
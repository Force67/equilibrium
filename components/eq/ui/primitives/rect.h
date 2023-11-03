// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace eq::ui {

template <typename T = float>
struct Rect {
  T left;
  T top;
  T right;
  T bottom;

  static constexpr Rect<T> MakeXYWH(T x, T y, T width, T height) {
    return Rect<T>(x, y, x + width, y + height);
  }

  // Default constructor
  constexpr Rect() : left(0), top(0), right(0), bottom(0) {}

  // Constructor using left, top, right, and bottom
  constexpr Rect(T left, T top, T right, T bottom)
      : left(left), top(top), right(right), bottom(bottom) {}

  // Constructor that sets all values to the same provided value
  constexpr Rect(T fill_value)
      : left(fill_value),
        top(fill_value),
        right(fill_value),
        bottom(fill_value) {}

  void MoveBy(T dx, T dy) {
    left += dx;
    right += dx;
    top += dy;
    bottom += dy;
  }

  bool IsPointInside(T x, T y) const {
    return x >= left && x <= right && y >= top && y <= bottom;
  }

  // workaround for now...
  template <typename T>
  inline T custom_max(const T& a, const T& b) {
    return (a > b) ? a : b;
  }

  template <typename T>
  inline T custom_min(const T& a, const T& b) {
    return (a < b) ? a : b;
  }

  Rect<T> Intersects(const Rect<T>& other) const {
    return Rect<T>(custom_max(left, other.left), custom_max(top, other.top),
                   custom_min(right, other.right),
                   custom_min(bottom, other.bottom));
  }

  T x() const { return left; }
  T y() const { return top; }
  T width() const { return right - left; }
  T height() const { return bottom - top; }
  T area() const { return width() * height(); }

  bool contains(const Rect<T>& other) const {
    return left <= other.left && right >= other.right && top <= other.top &&
           bottom >= other.bottom;
  }

  bool overlaps(const Rect<T>& other) const {
    return left < other.right && right > other.left && top < other.bottom &&
           bottom > other.top;
  }

  // Comparison operators
  bool operator==(const Rect& other) const {
    return left == other.left && top == other.top && right == other.right &&
           bottom == other.bottom;
  }

  bool operator!=(const Rect& other) const { return !(*this == other); }

  // Check if the rectangle is empty (all values are 0)
  bool empty() const {
    return left == 0 && top == 0 && right == 0 && bottom == 0;
  }
};

// Define an integer version of the rectangle
using IRect = Rect<i32>;

}  // namespace ui

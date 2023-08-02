// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace ui {
template <typename T = float>
struct Rect {
  T left;
  T top;
  T right;
  T bottom;

  Rect() : x(0), y(0), width(0), height(0) {}
  Rect(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
  Rect(T fill_value)
      : x(fill_value), y(fill_value), width(fill_value), height(fill_value) {}

  bool operator==(const Rect& other) const {
    return x == other.x && y == other.y && width == other.width &&
           height == other.height;
  }

  bool operator!=(const Rect& other) const { return !(*this == other); }

  bool empty() const { return x == 0 && y == 0 && width == 0 && height == 0; }
};

using IRect = Rect<i32>;
}  // namespace ui
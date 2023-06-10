// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace ui {
template <typename T = float>
struct Rect {
  T x;
  T y;
  T width;
  T height;

  Rect() : x(0), y(0), width(0), height(0) {}

  Rect(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
};

using IRect = Rect<i32>;
}  // namespace ui
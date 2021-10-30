// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {
template <typename T>
struct Vec2 {
  T a;
  T b;
};

using Vec2i = Vec2<int>;
}  // namespace base
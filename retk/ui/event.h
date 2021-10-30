// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <base/math/vec2.h>

namespace ui {
struct MouseEvent {
  base::Vec2i pos;
  base::Vec2i old_pos;
  base::Vec2i screen_pos;
  base::Vec2i old_pos_screen;
  uint32_t button, buttons;
};


}  // namespace ui
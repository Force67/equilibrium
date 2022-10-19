// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "label.h"
#include <core/SkCanvas.h>

namespace ui {

void Label::Paint(SkCanvas& canvas, const base::Vec2i) {
  const auto& s = state();

  canvas.drawString(s.text, bounds().a, bounds().b, *font_, paint);
}

}  // namespace ui

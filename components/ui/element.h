// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <base/math/vec2.h>

#include <core/SkTypes.h>
#include <core/SkString.h>
#include <core/SkPaint.h>

class SkCanvas;
class SkFont;

namespace ui {
enum struct Anchor {
  LEFT,
  CENTER,
  RIGHT,
  TOP_LEFT,
  TOP_CENTER,
  TOP_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_CENTER,
  BOTTOM_RIGHT,
};

class Element {
 public:
  Element();
  virtual ~Element() = default;

  virtual void Paint(SkCanvas& canvas, const base::Vec2i){};

  const base::Vec2i bounds() const { return bounds_; }

 private:
  bool active_ = false;
  int flags_ = 0;

 protected:
  // max-min size
  base::Vec2i max_bounds_;
  base::Vec2i min_bounds_;
  base::Vec2i bounds_;
  SkFont* font_{nullptr};
  SkPaint paint;

  std::vector<Element*> children_;
};

}  // namespace ui
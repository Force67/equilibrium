// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <base/math/vec2.h>
#include <ui/primitives/point.h>

namespace ui {

class Element {
 public:
  inline Element() {
    screen_pos = {0.f, 0.f};
    bounds = {12.f, 12.f};
  }

  std::vector<Element*> children_;

  size_t child_count() const { return children_.size(); }

  ui::FPoint screen_pos;
  ui::FPoint bounds;
};

class Layout {
 public:
  virtual ~Layout() = default;

  // Sets the current layout
  virtual void Build() = 0;
};
}  // namespace ui
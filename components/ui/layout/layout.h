// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <core/SkPoint.h>
#include <base/math/vec2.h>

namespace ui {

class Element {
 public:
  inline Element() {
    screen_pos = {0.f, 0.f};
    bounds = {12.f, 12.f};
  }

  std::vector<Element*> children_;

  size_t child_count() const { return children_.size(); }

  SkPoint screen_pos;
  SkPoint bounds;
};

class Layout {
 public:
  virtual ~Layout() = default;

  // Sets the current layout
  virtual void Build() = 0;
};
}  // namespace ui
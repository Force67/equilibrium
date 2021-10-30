// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <base/math/vec2.h>

namespace ui {

class Element {
 public:
  std::vector<Element*> children_;
};

class LayoutBase {
 public:
  virtual ~LayoutBase() = default;

  // current bounds
  virtual base::Vec2i Size() = 0;

  // Sets the current layout
  virtual void Apply() = 0;
};
}  // namespace ui
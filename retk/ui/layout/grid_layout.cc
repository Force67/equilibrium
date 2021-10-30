// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "grid_layout.h"

namespace ui {

base::Vec2i ui::GridLayout::Size() {
  if (children_.size() == 0)
    return {};

  for (auto& child : children_) {
  
  }

  return {};
}

void ui::GridLayout::Apply() {}

}  // namespace ui

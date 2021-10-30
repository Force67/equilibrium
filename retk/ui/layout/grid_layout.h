// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "layout_base.h"

namespace ui {
class GridLayout final : public LayoutBase, public Element {
 public:
  base::Vec2i Size() override;
  void Apply() override;

 private:
  base::Vec2i cells_;
  base::Vec2i spacing_;
};
}  // namespace ui
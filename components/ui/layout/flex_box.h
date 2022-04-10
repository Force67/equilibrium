// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "layout.h"
#include <core/SkCanvas.h>

namespace ui {
// follows the css grid spec:
// READ: https://developer.mozilla.org/en-US/docs/Learn/CSS/CSS_layout/Grids
class Flexbox final : public Layout, public Element {
 public:
  // in which direction the flex box will layout your items in
  enum class FlexDirection { kRow, kRowReverse, kColumn, kColumnReverse };

  enum class JustifyContent {
    kFlexStart,
    kFlexEnd,
    kCenter,
    kSpaceAround,
    kSpaceBetween,
    KSpaceEvenly
  };

  enum class AlignItems {
      kStrech,
      kFlexStart,
      kFlexEnd,
      kCenter
  };

  void Build() override;

 private:
 private:
  FlexDirection flex_direction_;
  JustifyContent justify_content_;
  AlignItems item_alignment_;
};
}  // namespace ui
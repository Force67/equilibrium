// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "layout.h"
#include <core/SkCanvas.h>

namespace ui {
// follows the css grid spec:
// READ: https://developer.mozilla.org/en-US/docs/Learn/CSS/CSS_layout/Grids
class GridLayout final : public Layout, public Element {
 public:
  GridLayout();

  template <typename... Ts>
  GridLayout& columns(Ts... c) {
    static constexpr size_t N = sizeof...(Ts);
    columns_.reserve(N);
    (columns_.emplace_back(c), ...);
    return *this;
  }

  template <typename... Ts>
  GridLayout& rows(Ts... c) {
    static constexpr size_t N = sizeof...(Ts);
    rows_.reserve(N);
    (rows_.emplace_back(c), ...);
    return *this;
  }

  GridLayout& gap(SkScalar gap) {
    row_gap_ = gap;
    col_gap_ = gap;
    return *this;
  }

  GridLayout& row_gap(SkScalar gap) {
    row_gap_ = gap;
    return *this;
  }

  GridLayout& column_gap(SkScalar gap) {
    col_gap_ = gap;
    return *this;
  }

  size_t column_count() const { return columns_.size(); }
  size_t row_count() const { return rows_.size(); }

  void Build() override;

 private:

 private:
  std::vector<SkScalar> columns_;
  std::vector<SkScalar> rows_;
  SkScalar row_gap_;
  SkScalar col_gap_;
};
}  // namespace ui
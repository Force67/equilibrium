// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "grid_layout.h"

namespace eq::ui {

GridLayout::GridLayout() {
  gap(10.f);
}

void GridLayout::Build() {
  const size_t CS = column_count();
  const size_t RS = row_count();
  BUGCHECK(CS);

  // calculate sizes
  if (CS) {
    // if the property was ever set we now compute
    // the sizes of the elements:
    size_t cc = 0;
    size_t rc = 0;
    for (size_t j = 0; j < children_.size(); j++) {
      Element* e = children_[j];

      // poor man's gcd
      if (j % CS == 0) {
        cc = 0;
        if (j > 0)
          rc++;
      } else
        cc++;

      e->bounds.x = columns_[cc];

      // rows again are optional
      if (RS)
        e->bounds.y = rows_[rc];
    }
  }

  // calculate positions
  ui::FPoint offset = {};
  f32 lastY = 0.f;
  for (size_t j = 0; j < children_.size(); j++) {
    Element* e = children_[j];
    // column break
    if (j % CS == 0) {
      offset.x = 0.f;

      // on line break we advance offset by last row height
      if (j > 0) {
        offset.y += lastY;
      }

      lastY = row_gap_ + e->bounds.y;
    }

    e->screen_pos.x = offset.x;
    e->screen_pos.y = offset.y;
    offset.x += col_gap_ + e->bounds.x;
  }
}
}  // namespace eq::ui

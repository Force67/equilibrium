// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "grid_layout.h"

namespace ui {

GridLayout::GridLayout() {
  gap(10.f);
}

void GridLayout::Build() {
  const size_t CS = column_count();
  const size_t RS = row_count();
  TK_BUGCHECK(CS);

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
        if (j > 0) rc++;
      } else
        cc++;

      e->bounds.fX = columns_[cc];

      // rows again are optional
      if (RS)
        e->bounds.fY = rows_[rc];
    }
  }

  // calculate positions
  SkPoint offset = {};
  SkScalar lastY = 0.f;
  for (size_t j = 0; j < children_.size(); j++) {
    Element* e = children_[j];
    // column break
    if (j % CS == 0) {
      offset.fX = 0.f;

      // on line break we advance offset by last row height
      if (j > 0) {
        offset.fY += lastY;
      }

      lastY = row_gap_ + e->bounds.y();
    }

    e->screen_pos.fX = offset.fX;
    e->screen_pos.fY = offset.fY;
    offset.fX += col_gap_ + e->bounds.x();
  }
}
}  // namespace ui

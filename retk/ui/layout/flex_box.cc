// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include "flex_box.h"

namespace ui {

void Flexbox::Build() {
  switch (flex_direction_) {
    case FlexDirection::kRow:
    case FlexDirection::kRowReverse: {

      break;
    }
    case FlexDirection::kColumn:
    case FlexDirection::kColumnReverse: {
      break;
    }
    default:
      TK_DCHECK(false);
  }
}
}  // namespace ui
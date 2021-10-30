// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "stateful_element.h"

// https://github.com/skui-org/skui/blob/master/gui/element/label.h%2B%2B

namespace ui {

struct LabelState {
  SkString text{"<unnamed>"};
  Anchor anchor{Anchor::LEFT};
};

class Label : public StatefulElement<LabelState> {
 public:
  ~Label() = default;

  void Paint(SkCanvas& canvas, const base::Vec2i) override;
};
}  // namespace ui
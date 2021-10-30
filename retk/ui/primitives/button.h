// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/element.h"

namespace ui {

class Button : public Element {
 public:
  Button();


 void Paint(SkCanvas& canvas, const base::Vec2i) override;
};
}  // namespace ui
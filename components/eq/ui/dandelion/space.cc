// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "space.h"
#include "screen.h"

namespace user_interface {

Space::Space() {
  gamma_screen()->RegisterSpace(this);
}

Space::~Space() {
  gamma_screen()->UnRegisterSpace(this);
}

void Space::SetSize(float w, float h) {
  w = std::max(0.0f, w);
  h = std::max(0.0f, h);

  if (geom_.width != w || geom_.height != h) {
    geom_.width = w;
    geom_.height = h;
    OnResize();
  }
}
}
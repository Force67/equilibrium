// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "screen.h"

namespace user_interface {

static Screen* g_screen = nullptr;

Screen::Screen() {
  g_screen = this;
}

Screen::~Screen() {
  g_screen = nullptr;
}

void Screen::RegisterSpace(Space* space) {
  spaces_.push_back(space);
}

void Screen::UnRegisterSpace(Space* space) {

}

Screen* gamma_screen() {
  //TODO: Dcheck
  return g_screen;
}
}

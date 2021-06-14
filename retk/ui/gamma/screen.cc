// Copyright (C) 2021 Force67 <github.com/Force67>.
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
  auto it = std::find_if(spaces_.begin(), spaces_.end(),
                         [&](Space* s) { return s == space; });

  if (it != spaces_.end()) {
    spaces_.erase(it);
  }
}

Screen* gamma_screen() {
  //TODO: Dcheck
  return g_screen;
}
}
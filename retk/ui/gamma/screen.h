// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>

namespace user_interface {
class Space;

class Screen {
 public:
  Screen();
  ~Screen();

  void RegisterSpace(Space*);
  void UnRegisterSpace(Space*);

 private:
  std::vector<Space*> spaces_;
};

Screen* gamma_screen();
}
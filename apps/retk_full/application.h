// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "glfw_window.h"

class Application {
 public:
  Application();
  ~Application();

  int Exec();

 private:
  WindowGlfw main_window_;
};
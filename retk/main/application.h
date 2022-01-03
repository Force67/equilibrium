// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "glfw_window.h"

#include <ui/imgui/imgui_context.h>
#include <ui/skia/layer/imgui_layer.h>

class Application {
 public:
  Application();
  ~Application();

  int Exec();

 private:
  ui::DearImGuiContext im_ctx_;
  ui::ImguiSkiaLayer layer_;
  WindowGlfw main_window_;
};
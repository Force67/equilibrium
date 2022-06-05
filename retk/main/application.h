// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <ui/imgui/imgui_context.h>
#include <ui/skia/layer/imgui_layer.h>
#include <ui/platform/native_window.h>

class Application {
 public:
  Application();
  ~Application();

  int Exec();

 private:
  // Order here matters
  ui::DearImGuiContext im_ctx_;
  ui::ImguiSkiaLayer layer_;
  std::unique_ptr<ui::NativeWindow> window_;
};
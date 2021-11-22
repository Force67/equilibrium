// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// ImGui render layer implementation on top of SKIA.
#pragma once

#include <core/SkPaint.h>

class SkCanvas;

namespace ui {

class DearImGuiContext;

class ImguiSkiaLayer final {
 public:
  explicit ImguiSkiaLayer(DearImGuiContext&);

  void Draw(SkCanvas* canvas);

 private:
  DearImGuiContext& im_data_;
  SkPaint font_paint_;
};
}  // namespace ui
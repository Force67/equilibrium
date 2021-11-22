// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// ImGui render layer implementation on top of SKIA.
#pragma once

class SkCanvas;

namespace ui {

class ImguiSkiaLayer final {
 public:
  static void Draw(SkCanvas* canvas);
};
}  // namespace ui
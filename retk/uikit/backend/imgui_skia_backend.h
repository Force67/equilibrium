// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class SkCanvas;

namespace uikit {
	
class ImguiSkiaBackend {
 public:
  ImguiSkiaBackend();
  ~ImguiSkiaBackend();

  void Draw(SkCanvas *canvas);
};
}
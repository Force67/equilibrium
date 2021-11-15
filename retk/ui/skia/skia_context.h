// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Skia backend implementation.
#pragma once

#include <memory>
#include <core/SkRefCnt.h>

class SkSurface;
class GrDirectContext;

namespace ui {

	struct ContextCreateInfo {
  int width, height;
};

class SkiaContext {
 public:
  SkiaContext();
  ~SkiaContext();

  bool Initialize(const ContextCreateInfo&);

  void SetDpiAware(void *window_handle);

  SkCanvas* canvas() { return surface_->getCanvas(); }
  void Flush() { surface_->flush(); }

 private:
  // keep surface first as it needs to be destroyed first
  SkSurface *surface_;
  GrDirectContext *gpu_context_;
};
}  // namespace ui
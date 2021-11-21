// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Skia backend implementation.
#pragma once

#include <memory>

#include <core/SkRefCnt.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>

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

  void SetDpiAware(void* window_handle, bool scale = true);

  void Resize(SkPoint new_size);

  // get the current canvas
  SkCanvas* canvas() { return surface_->getCanvas(); }
  SkPoint dpi_scale() { return dpi_scale_; }

  inline void Flush() { surface_->flush(); }

  void RestoreScaling();

 private:
  // keep surface first as it needs to be destroyed first
  SkSurface* surface_;
  GrDirectContext* gpu_context_;

  SkPoint dpi_scale_;
};
}  // namespace ui
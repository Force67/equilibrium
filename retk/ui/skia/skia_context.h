// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Skia backend implementation.
#pragma once

#include <memory>
#include <core/SkRefCnt.h>

class SkSurface;
class GrDirectContext;

namespace ui {

class SkiaContext {
 public:
  SkiaContext(sk_sp<SkSurface>, sk_sp<GrDirectContext>);
  ~SkiaContext();

  void SetDpiAware(void *window_handle);

  SkCanvas* canvas() { return surface_->getCanvas(); }
  void Flush() { surface_->flush(); }

 private:
  // keep surface first as it needs to be destroyed first
  sk_sp<SkSurface> surface_;
  sk_sp<GrDirectContext> gpu_context_;
};

struct SkiaCreateInfo {
  int width, height;
};

std::unique_ptr<SkiaContext> CreateSkiaContext(const SkiaCreateInfo&);
}  // namespace ui
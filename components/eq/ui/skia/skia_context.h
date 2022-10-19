// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Skia backend implementation.
#pragma once

#include <memory>

#include <core/SkRefCnt.h>
#include <core/SkCanvas.h>
#include <core/SkSurface.h>

class GrDirectContext;

namespace ui {

enum class GrcApi { kUnknown = -1, kOpenGl, kVulkan };

struct ContextCreateInfo {
  GrcApi gr_api_type;
  SkIPoint dimensions;
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

  // changed flush() to flushAndSubmit()
  inline void Flush() { surface_->flushAndSubmit(); }

  void RestoreScaling();

 private:
  void RebuildGlContext(SkPoint);
  void RebuildVkContext(SkPoint);

 private:
  GrcApi grc_api_{GrcApi::kUnknown};
  // keep surface first as it needs to be destroyed first
  SkSurface* surface_;
  GrDirectContext* gpu_context_;

  SkPoint dpi_scale_{};
};
}  // namespace ui
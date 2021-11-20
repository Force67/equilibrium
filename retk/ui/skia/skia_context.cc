// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Skia backend implementation.

#include <core/SkGraphics.h>
#include <core/SkSurface.h>
#include <core/SkCanvas.h>

#include <gpu/gl/GrGLInterface.h>
#include <gpu/GrContextOptions.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrBackendSurface.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <base/logging.h>

#include "display/dpi.h"
#include "skia/skia_context.h"

namespace ui {

SkiaContext::SkiaContext() : surface_(nullptr), gpu_context_(nullptr) {}

SkiaContext::~SkiaContext() {
  delete surface_;
  delete gpu_context_;
}

bool SkiaContext::Initialize(const ContextCreateInfo& info) {
  GrContextOptions options;
  gpu_context_ = GrDirectContext::MakeGL(options).release();

  Resize({static_cast<float>(info.width), static_cast<float>(info.height)});
  return surface_ != nullptr;
}

void SkiaContext::Resize(SkPoint screen_size) {
  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0;  // assume default framebuffer
  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  //(replace line below with this one to enable correct color spaces)
  // framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;
#if 0
  SkColorType colorType;
  if (kRGBA_8888_GrPixelConfig == kSkia8888_GrPixelConfig) {
    colorType = kRGBA_8888_SkColorType;
  } else {
    colorType = kBGRA_8888_SkColorType;
  }
#endif
  SkColorType colorType = kBGRA_8888_SkColorType;

  // should be stored
  GrBackendRenderTarget backendRenderTarget(screen_size.fX, screen_size.fY,
                                            0,  // sample count
                                            0,  // stencil bits
                                            framebufferInfo);

  //(replace line below with this one to enable correct color spaces) sSurface =
  // SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget,
  // kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(),
  // nullptr).release();
  if (surface_) {
    delete surface_;
    surface_ = nullptr;

  }

  surface_ = SkSurface::MakeFromBackendRenderTarget(
                 gpu_context_, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                 colorType, nullptr, nullptr)
                 .release();

  TK_DCHECK(canvas());
}

void SkiaContext::SetDpiAware(void* window_handle, bool scale) {
  TK_DCHECK(window_handle && surface_);
  // enable dpi scaling.
  // TODO: handle monitor switching
  // see: https://gitter.im/AvaloniaUI/Avalonia?at=5802746c891a53016311d46f
  SkCanvas* canvas = surface_->getCanvas();
  canvas->restoreToCount(0);
  canvas->save();

  if (scale) {
    dpi_scale_ = GetCurrentDpiScalingFactor(window_handle);
    canvas->scale(dpi_scale_.fX, dpi_scale_.fY);
  }
  // canvas->resetMatrix();
}

void SkiaContext::RestoreScaling() {
  canvas()->scale(dpi_scale_.fX, dpi_scale_.fY);
}
}  // namespace ui
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

SkiaContext::SkiaContext(sk_sp<SkSurface> surface,
                         sk_sp<GrDirectContext> gr_context)
    : gpu_context_(std::move(gr_context)), surface_(std::move(surface)) {}

SkiaContext::~SkiaContext() {}

void SkiaContext::SetDpiAware(void* window_handle) {
  TK_DCHECK(window_handle && surface_);
  // enable dpi scaling.
  // TODO: handle monitor switching
  // see: https://gitter.im/AvaloniaUI/Avalonia?at=5802746c891a53016311d46f
  SkCanvas* canvas = surface_->getCanvas();
  canvas->restoreToCount(0);
  canvas->save();

  SkPoint dpi = GetCurrentDpiScalingFactor(window_handle);
  canvas->scale(dpi.fX, dpi.fY);
  // canvas->resetMatrix();

  SkMatrix transform{};
  transform = SkMatrix() * SkMatrix::Scale(1.25, 1.25);
  canvas->setMatrix(transform);

  // apply global scaling
  // auto mat = canvas->getTotalMatrix();
  // mat = mat * SkMatrix::Scale(1.25, 1.25);
  // canvas->setMatrix(mat);
}

// TODO: Possibly move this to a factory..
std::unique_ptr<SkiaContext> CreateSkiaContext(
    const SkiaCreateInfo& create_info) {
  GrContextOptions options;
  auto gr_context = GrDirectContext::MakeGL(nullptr, options);

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

  GrBackendRenderTarget backendRenderTarget(create_info.width,
                                            create_info.height,
                                            0,  // sample count
                                            0,  // stencil bits
                                            framebufferInfo);

  //(replace line below with this one to enable correct color spaces) sSurface =
  // SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget,
  // kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(),
  // nullptr).release();
  auto surface = SkSurface::MakeFromBackendRenderTarget(
      gr_context.get(), backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
      colorType, nullptr, nullptr);
  if (surface == nullptr)
    return nullptr;

  return std::make_unique<SkiaContext>(std::move(surface),
                                       std::move(gr_context));
}
}  // namespace ui
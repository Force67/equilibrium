// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <core/SkGraphics.h>
#include <core/SkSurface.h>
#include <core/SkCanvas.h>

#include <gpu/gl/GrGLInterface.h>
#include <gpu/GrContextOptions.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrBackendSurface.h>
#include <base/logging.h>

#include "skia_context_factory.h"

namespace ui {
// TODO: Possibly move this to a factory..
base::UniquePointer<SkiaContext> CreateSkiaContext(
    const ContextCreateInfo& create_info) {
  auto context = base::MakeUnique<SkiaContext>();

  if (!context->Initialize(create_info)) {
    __debugbreak();
  }

  return base::move(context);
}
}
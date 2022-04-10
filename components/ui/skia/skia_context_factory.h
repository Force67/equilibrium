// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "skia_context.h"

namespace ui {

std::unique_ptr<SkiaContext> CreateSkiaContext(const ContextCreateInfo&);
}  // namespace ui
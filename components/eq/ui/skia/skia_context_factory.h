// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "skia_context.h"

#include <base/memory/unique_pointer.h>

namespace ui {

base::UniquePointer<SkiaContext> CreateSkiaContext(const ContextCreateInfo&);
}  // namespace ui
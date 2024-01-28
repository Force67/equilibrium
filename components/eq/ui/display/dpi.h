// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <eq/ui/primitives/point.h>

namespace eq::ui {

eq::ui::Point<f32> GetCurrentDpi(void* os_window);
eq::ui::Point<f32> GetCurrentDpiScalingFactor(void* os_window);
}  // namespace eq::ui
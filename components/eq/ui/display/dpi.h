// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <ui/primitives/point.h>

namespace ui {

ui::Point<f32> GetCurrentDpi(void* os_window);
ui::Point<f32> GetCurrentDpiScalingFactor(void* os_window);
}
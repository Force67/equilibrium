// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <core/SkPoint.h>

namespace ui {

SkPoint GetCurrentDpi(void* os_window);
SkPoint GetCurrentDpiScalingFactor(void* os_window);
}
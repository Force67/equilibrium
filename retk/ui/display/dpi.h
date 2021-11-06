// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <core/SkPoint.h>

namespace ui {

SkPoint GetCurrentDpiScalingFactor(void* os_window);
}
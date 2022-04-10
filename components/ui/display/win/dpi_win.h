// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <base/win/minwin.h>

namespace ui {
constexpr float kDefaultWindowsDPI = 96.f;

HMONITOR GetCurrentMonitorHandle(HWND hwnd);
SkPoint GetMonitorDpi(HMONITOR monitor_handle);

inline SkScalar GetDPIFactor(SkScalar dpi) {
  return dpi / kDefaultWindowsDPI;
}
}  // namespace ui
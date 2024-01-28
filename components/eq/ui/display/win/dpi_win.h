// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.
#pragma once

#include <base/win/minwin.h>

namespace eq::ui {
constexpr f32 kDefaultWindowsDPI = 96.f;

HMONITOR GetCurrentMonitorHandle(HWND hwnd);
eq::ui::FPoint GetMonitorDpi(HMONITOR monitor_handle);

inline eq::ui::FPoint GetDPIFactor(f32 dpi) {
  return dpi / kDefaultWindowsDPI;
}
}  // namespace ui
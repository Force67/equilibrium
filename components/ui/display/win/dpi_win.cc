// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// High dpi scaling utilities.

#include <Windows.h>
#include <ShellScalingApi.h>

#include "display/dpi.h"

namespace ui {

HMONITOR GetCurrentMonitorHandle(HWND hwnd) {
  return MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
}

SkPoint GetMonitorDpi(HMONITOR monitor_handle) {
  // https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getscalefactorformonitor
  // https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-3/
  // usage with skia:
  // https://gitter.im/AvaloniaUI/Avalonia?at=5802746c891a53016311d46f usage
  // with imgui: https://ourmachinery.com/post/dpi-aware-imgui/ Don't forget to
  // add the manifest to your application that specifies dpi awareness note
  // however that this method is unsupported on systems running windows 8 or
  // older but we don't provide fall back for these cases since we dropped
  // windows 8 support. DPI
  // https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-should-i-handle-dpi-in-my-application
  UINT x, y;
  x = y = USER_DEFAULT_SCREEN_DPI;
  GetDpiForMonitor(monitor_handle, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI, &x, &y);

  return SkPoint::Make(static_cast<float>(x), static_cast<float>(y));
}

// TODO: this is not UWP compatible!
// https://github.com/chromium/chromium/blob/72ceeed2ebcd505b8d8205ed7354e862b871995e/ui/display/win/screen_win.cc#L66
SkPoint GetCurrentDpi(void* os_window) {
  HMONITOR monitor_handle =
      GetCurrentMonitorHandle(static_cast<HWND>(os_window));
  return GetMonitorDpi(monitor_handle);
}

SkPoint GetCurrentDpiScalingFactor(void* os_window) {
  auto raw_dpi = GetCurrentDpi(os_window);
  constexpr float kDefaultDpi = static_cast<float>(USER_DEFAULT_SCREEN_DPI);
  return {raw_dpi.fX / kDefaultDpi, raw_dpi.fY / kDefaultDpi};
}
}  // namespace ui
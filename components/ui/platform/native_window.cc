// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.

#include <ui/platform/native_window.h>

#if defined(OS_WIN)
#include <ui/platform/win/native_window_win32.h>
#endif

namespace ui {
base::UniquePointer<NativeWindow> MakeWindow(const base::StringRefU8 title) {
  // TODO(Vince): a bunch of compositor logic for linux
  // TODO(Vince): UWP window support.
#if defined(OS_WIN)
  return base::move(base::MakeUnique<NativeWindowWin32>(title));
#endif
}
}  // namespace ui
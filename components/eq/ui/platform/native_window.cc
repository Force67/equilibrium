// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.

#include "native_window.h"

#if defined(OS_WIN)
#include "win/native_window_win32.h"
#endif

namespace ui {
base::UniquePointer<NativeWindow> MakeWindow(const base::StringRefU8 title,
                                             NativeWindow::Delegate* d) {
  // TODO(Vince): a bunch of compositor logic for linux
  // TODO(Vince): UWP window support.
#if defined(OS_WIN)
  return base::move(base::MakeUnique<NativeWindowWin32>(
      title, reinterpret_cast<WindowDelegateWin*>(d)));
#endif
}
}  // namespace ui
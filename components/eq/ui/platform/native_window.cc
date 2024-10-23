// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.

#include "native_window.h"
#include "base/memory/unique_pointer.h"

#if defined(OS_WIN)
#include "win/native_window_win32.h"
#endif

#if defined(OS_LINUX)
#include "linux/wayland_window_linux.h"
#endif

namespace eq::ui {
base::UniquePointer<NativeWindow> MakeWindow(const base::StringRefU8 title,
                                             NativeWindow::Delegate* d) {
  // NOTE(Vince): We dont support UWP, since its also not even required on xbox anymore..
#if defined(OS_WIN)
  return base::move(base::MakeUnique<NativeWindowWin32>(
      title, reinterpret_cast<WindowDelegateWin*>(d)));
#endif

#if defined(OS_LINUX)
  return base::move(base::MakeUnique<NativeWindowWayland>(title));
#endif
}
}  // namespace eq::ui

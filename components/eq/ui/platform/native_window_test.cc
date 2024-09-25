// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#if defined (OS_WIN)
#include "win/message_pump_win.h"
#include "win/native_window_win32.h"
#endif

namespace {
#if defined (OS_WIN)
TEST(NativeWindow, Create) {
  eq::ui::NativeWindowWin32 win_window(u8"WindowTest", nullptr);
  EXPECT_TRUE(win_window.Init(HWND_DESKTOP, {}, eq::ui::NativeWindow::CreateFlags::kNone));

  win_window.Show();

  eq::ui::MessagePumpWin mp;
 // while (mp.Update()) {
 //   mp.Pump();
 // }
}
#endif
}  // namespace

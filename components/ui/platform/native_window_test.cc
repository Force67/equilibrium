// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <platform/win/message_pump_win.h>
#include <platform/win/native_window_win32.h>

namespace {
TEST(NativeWindow, Create) {
  ui::NativeWindowWin32 win_window(u8"WindowTest", nullptr);
  EXPECT_TRUE(win_window.Init(HWND_DESKTOP, {}));

  win_window.Show();


  ui::MessagePumpWin mp;
 // while (mp.Update()) {
 //   mp.Pump();
 // }
}
}  // namespace

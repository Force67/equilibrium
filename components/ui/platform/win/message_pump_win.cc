// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform message loop.

#include <Windows.h>
#include <ui/platform/win/message_pump_win.h>

namespace ui {
bool MessagePumpWin::Update() {
  return GetMessageW(&msg_, nullptr, 0, 0) != 0;
}

void MessagePumpWin::Pump() {
  TranslateMessage(&msg_);
  DispatchMessageW(&msg_);
}
}  // namespace ui
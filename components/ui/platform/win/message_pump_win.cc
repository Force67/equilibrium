// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform message loop.

#include <Windows.h>
#include <ui/platform/win/message_pump_win.h>

namespace ui {
bool MessagePumpWin::UpdateBlocking() {
  return ::GetMessageW(&msg_, nullptr, 0, 0) != 0;
}

bool MessagePumpWin::Peek() {
  return ::PeekMessageW(&msg_, nullptr, 0, 0, PM_REMOVE) != 0;
}

void MessagePumpWin::Pump() {
  ::TranslateMessage(&msg_);
  ::DispatchMessageW(&msg_);
}
}  // namespace ui
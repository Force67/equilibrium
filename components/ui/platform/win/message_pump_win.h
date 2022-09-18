// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform message loop.
#pragma once

#include <Windows.h>
#include <base/win/minwin.h>

namespace ui {

class MessagePumpWin {
 public:
  // https://stackoverflow.com/questions/2850186/why-peekmessage-before-getmessage
  bool UpdateBlocking();
  bool Peek();

  void Pump();

  bool WantsToQuit() const { return msg_.message == WM_QUIT; }

 private:
  MSG msg_{};
};
}  // namespace ui
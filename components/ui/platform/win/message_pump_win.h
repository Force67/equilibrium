// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform message loop.
#pragma once

#include <Windows.h>
#include <base/win/minwin.h>

namespace ui {

class MessagePumpWin {
 public:
  bool Update();
  void Pump();

 private:
  MSG msg_{};
};
}  // namespace ui
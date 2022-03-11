// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform message loop.
#pragma once

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
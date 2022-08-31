// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// retained mode ui.

#include <base/memory/unique_pointer.h>
#include <ui/platform/native_window.h>
#include <ui/platform/win/native_window_win32.h>

namespace main {

class MainWindow {
 public:
  MainWindow();
  ~MainWindow();

  void Initialize();

 private:
  base::UniquePointer<ui::NativeWindowWin32> native_window_;
};
}  // namespace main
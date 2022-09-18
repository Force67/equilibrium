// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// retained mode ui.

#include <ui/platform/native_window.h>

namespace main {

class MainWindow {
 public:
  MainWindow();
  ~MainWindow();

  void Initialize();

 private:
  base::UniquePointer<ui::NativeWindow> native_window_;
};
}  // namespace main
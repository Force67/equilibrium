// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <main_window.h>
#include <ui/platform/native_window.h>

namespace main {

namespace {
constinit base::StringRefU8 kWindowTitle{u8"RETK",
                                         sizeof(u8"RETK") / sizeof(char8_t), true};
}

MainWindow::MainWindow() : native_window_(ui::MakeWindow(kWindowTitle)) {}
MainWindow::~MainWindow() {}

void MainWindow::Initialize() {
  native_window_->Init(nullptr, SkIRect::MakeXYWH(0, 0, 1920, 1080));
  native_window_->Show();
}
}  // namespace main
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <debug_window.h>
#include <ui/platform/native_window.h>

namespace main {

namespace {
constinit base::StringRefU8 kWindowTitle{
    u8"Debug Window", sizeof(u8"Debug Window") / sizeof(char8_t), true};
}

DebugWindow::DebugWindow() : native_window_(ui::MakeWindow(kWindowTitle)) {}
DebugWindow::~DebugWindow() {}

void DebugWindow::Initialize() {
  native_window_->Init(nullptr, SkIRect::MakeXYWH(0, 0, 1280, 720),
                       ui::NativeWindow::CreateFlags::kNone);
  native_window_->Show();
}
}  // namespace main
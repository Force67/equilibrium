// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>

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
  native_window_->Init(nullptr, SkIRect::MakeXYWH(0, 0, 1920, 1080),
                       ui::NativeWindow::CreateFlags::kNone);
  native_window_->Show();

  #if 0
  BUGCHECK(vk_instance_.Create());

  surface_.Make(vk_instance_.instance());

  BUGCHECK(surface_->Initialize(::GetModuleHandleW(nullptr), native_window_->os_handle(),
                      gpu::vulkan::VulkanSurface::Format::Default));
  #endif
}
}  // namespace main
// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.
#pragma once

#include <eq/ui/primitives/rect.h>
#include <eq/ui/primitives/point.h>
#include <eq/ui/platform/native_window.h>

extern "C" {
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shell;
struct wl_surface;
struct wl_shell_surface;
}

namespace eq::ui {
class NativeWindowWayland : public NativeWindow {
 public:
  NativeWindowWayland(const base::StringRefU8 title);
  ~NativeWindowWayland() override;

  bool Init(handle parent,
            const ui::IRect bounds,
            const CreateFlags,
            u8 icon_id = 102) override;

  void Destroy();

  bool SetTitle(const base::StringRefU8) override;
  void SendCommand(Command) override;

  handle os_handle() const override;
  const ui::IRect bounds() const override;

  void SetDelegate(eq::ui::NativeWindow::Delegate*) override;

 private:
  static void registry_listener(void* data,
                                struct wl_registry* registry,
                                unsigned int name,
                                const char* interface,
                                unsigned int version);

  wl_display* display_;
  wl_registry* registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  wl_surface* surface_;
  wl_shell_surface* shell_surface_;
  eq::ui::NativeWindow::Delegate* delegate_;
};
}  // namespace eq::ui

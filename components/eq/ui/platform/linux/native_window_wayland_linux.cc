// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/atomic.h>
#include <base/check.h>
#include <base/logging.h>
#include <wayland-client.h>

namespace ui {
NativeWindowWayland::NativeWindowWayland(const base::StringRefU8 title)
    : NativeWindow() {}

NativeWindowWayland::~NativeWindowWayland() { Destroy(); }

bool NativeWindowWayland::Init(handle native_parent_handle,
                               const ui::IRect bounds, const CreateFlags flags,
                               uint8_t icon_id) {
  display_ = wl_display_connect(nullptr);
  if (display_ == nullptr) return false;

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, nullptr);
  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  if (compositor == NULL || shell == NULL) {
    fprintf(stderr, "Can't find compositor or shell\n");
    exit(1);
  }

  surface = wl_compositor_create_surface(compositor);
  if (surface == NULL) {
    fprintf(stderr, "Can't create surface\n");
    exit(1);
  }

  shell_surface = wl_shell_get_shell_surface(shell, surface);
  if (shell_surface == NULL) {
    fprintf(stderr, "Can't create shell surface\n");
    exit(1);
  }

  // Initialize the window here using Wayland API
  // If a parent is provided, you can apply specific settings as needed
  return true;  // Return false if initialization fails
}

void NativeWindowWayland::Destroy() { wl_display_disconnect(display_); }

bool NativeWindowWayland::SetTitle(const base::StringRefU8 name) {
  return true;
}

void NativeWindowWayland::SendCommand(NativeWindow::Command command) {}

}  // namespace ui
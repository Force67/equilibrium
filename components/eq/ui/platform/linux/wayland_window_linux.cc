// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/atomic.h>
#include <base/check.h>
#include <base/logging.h>
#include <wayland-client.h>

#include "wayland_window_linux.h"

namespace eq::ui {
void NativeWindowWayland::registry_listener(void* data,
                                            struct wl_registry* registry,
                                            unsigned int name,
                                            const char* interface,
                                            unsigned int version) {
  NativeWindowWayland* window = static_cast<NativeWindowWayland*>(data);
  if (strcmp(interface, "wl_compositor") == 0) {
    window->compositor_ = static_cast<wl_compositor*>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_shell") == 0) {
    window->shell_ = static_cast<wl_shell*>(
        wl_registry_bind(registry, name, &wl_shell_interface, 1));
  }
}

NativeWindowWayland::NativeWindowWayland(const base::StringRefU8 title)
    : NativeWindow(title) {}

NativeWindowWayland::~NativeWindowWayland() {
  Destroy();
}

bool NativeWindowWayland::Init(handle native_parent_handle,
                               const ui::IRect bounds,
                               const CreateFlags flags,
                               uint8_t icon_id) {
  display_ = wl_display_connect(nullptr);
  if (display_ == nullptr)
    return false;

  registry_ = wl_display_get_registry(display_);
  static const struct wl_registry_listener registry_listeners = {
	registry_listener,
	NULL
  };

  wl_registry_add_listener(registry_, &registry_listeners, this);
  wl_display_dispatch(display_);
  wl_display_roundtrip(display_);

  if (compositor_ == nullptr || shell_ == nullptr) {
    BASE_LOG_ERROR("Can't find compositor or shell");
    return false;
  }

  surface_ = wl_compositor_create_surface(compositor_);
  if (surface_ == nullptr) {
    BASE_LOG_ERROR("Can't create surface");
    return false;
  }

  shell_surface_ = wl_shell_get_shell_surface(shell_, surface_);
  if (shell_surface_ == nullptr) {
    BASE_LOG_ERROR("Can't create shell surface");
    return false;
  }

  wl_shell_surface_set_title(shell_surface_, (const char*)title_.c_str());
  wl_shell_surface_set_toplevel(shell_surface_);

  if (bounds.width() > 0 && bounds.height() > 0) {
      #if 0
    wl_shell_surface_set_window_geometry(shell_surface_, bounds.x(), bounds.y(),
                                         bounds.width(), bounds.height());
    #endif
  }

  if (flags & CreateFlags::kResizable) {
    // Wayland doesn't have a direct API for setting resizable,
    // it's typically handled by the compositor
  }

  if (flags & CreateFlags::kMaximized) {
    wl_shell_surface_set_maximized(shell_surface_, nullptr);
  }

  // Icon setting is not directly supported in Wayland,
  // typically handled by the desktop environment

  return true;
}

void NativeWindowWayland::Destroy() {
  if (shell_surface_) {
    wl_shell_surface_destroy(shell_surface_);
    shell_surface_ = nullptr;
  }
  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = nullptr;
  }
  if (shell_) {
    wl_shell_destroy(shell_);
    shell_ = nullptr;
  }
  if (compositor_) {
    wl_compositor_destroy(compositor_);
    compositor_ = nullptr;
  }
  if (registry_) {
    wl_registry_destroy(registry_);
    registry_ = nullptr;
  }
  if (display_) {
    wl_display_disconnect(display_);
    display_ = nullptr;
  }
}

bool NativeWindowWayland::SetTitle(const base::StringRefU8 name) {
  if (shell_surface_) {
    // Docu states that this is fine and even expected to be utf8
    wl_shell_surface_set_title(shell_surface_, (const char*)name.c_str());
    return true;
  }
  return false;
}

void NativeWindowWayland::SendCommand(NativeWindow::Command command) {
  switch (command) {
    case Command::kClose:
      Destroy();
      break;
    case Command::kMaximize:
      if (shell_surface_) {
        wl_shell_surface_set_maximized(shell_surface_, nullptr);
      }
      break;
    case Command::kMinimize:
      // Wayland doesn't have a direct minimize command,
      // typically handled by the window manager
      break;
    case Command::kRestore:
      if (shell_surface_) {
        wl_shell_surface_set_toplevel(shell_surface_);
      }
      break;
    default:
      BASE_LOG_ERROR("Unsupported command: {}", static_cast<int>(command));
      break;
  }
}

}  // namespace eq::ui

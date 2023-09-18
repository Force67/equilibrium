// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.
#pragma once

#include <eq/ui/primitives/rect.h>
#include <eq/ui/primitives/point.h>
#include <eq/ui/platform/native_window.h>

struct wl_display;
struct wl_registry;

namespace ui {
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
private:
  wl_display* display_;
  wl_registry* registry_;
};
}
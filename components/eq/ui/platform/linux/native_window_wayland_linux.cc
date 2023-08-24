// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/check.h>
#include <base/atomic.h>
#include <base/logging.h>
#include <wayland-client.h>

namespace ui  {
	NativeWindowWayland::NativeWindowWayland(const base::StringRefU8 title) : NativeWindow() {

	}

	NativeWindowWayland::~NativeWindowWayland() {


	}

	bool NativeWindowWayland::Init(handle native_parent_handle,
                    const ui::IRect bounds,
                    const CreateFlags flags, uint8_t icon_id) {
    // Initialize the window here using Wayland API
    // If a parent is provided, you can apply specific settings as needed
    return true; // Return false if initialization fails
  }

  




}
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// retained mode ui.

#include <ui/platform/native_window.h>

#include <gpu/vulkan/vulkan_instance.h>
#include <gpu/vulkan/vulkan_surface.h>

namespace main {

class MainWindow {
 public:
  MainWindow();
  ~MainWindow();

  void Initialize();

 private:
  base::UniquePointer<ui::NativeWindow> native_window_;

  gpu::vulkan::VulkanInstance vk_instance_;

  base::LazyInstance<gpu::vulkan::VulkanSurface> surface_;
};
}  // namespace main
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <gpu/vulkan/vulkan_raii.h>

namespace gpu::vulkan {

class VulkanSurface {
 public:
  VulkanSurface(VkInstance_Cxx& instance) : instance_(instance){};
  ~VulkanSurface();

  enum class Format { Rgba32, Rgb16, Count, Default = Rgba32 };

  bool Initialize(void* os_hinstance, void* os_window_handle);

  inline VkSurfaceKHR instance() { return surface_handle_; }

 private:
  VkInstance_Cxx& instance_;
  VkSurfaceKHR surface_handle_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
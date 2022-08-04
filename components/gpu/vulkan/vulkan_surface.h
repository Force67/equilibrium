// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <gpu/vulkan/vulkan_raii.h>

namespace gpu::vulkan {

class VulkanSurface {
 public:
  VulkanSurface();
  ~VulkanSurface();

  enum class Format { Rgba32, Rgb16, Count, Default = Rgba32 };

  bool Initialize(VkInstance_Cxx& vki, void* os_hinstance, void* os_window_handle);

 private:
  VkSurfaceKHR surface_handle_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
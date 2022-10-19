// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/math/vec2.h>

#include <gpu/vulkan/vulkan_raii.h>

namespace gpu::vulkan {

class VulkanSurface;
class VulkanDeviceQueue;

class VulkanSwapChain {
 public:
  ~VulkanSwapChain();

  bool Create(VulkanSurface&,
              VulkanDeviceQueue& device_queue,
              u32 min_image_count,
              const VkSurfaceFormatKHR& format,
              const base::Vec2<u32> image_size);

 private:
  VkSwapchainKHR swapchain_handle_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
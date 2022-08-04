// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <gpu/vulkan/vulkan_raii.h>

namespace gpu::vulkan {

class VulkanSwapChain {
 public:
  bool Create();

 private:
  VkSwapchainKHR swapchain_handle_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
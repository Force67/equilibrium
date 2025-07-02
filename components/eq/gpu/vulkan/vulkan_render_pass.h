// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {
class VulkanDevice;

class VulkanRenderPass {
 public:
  VulkanRenderPass(const VulkanDevice& dev, VkFormat swapchainFormat);
  ~VulkanRenderPass();
  VkRenderPass handle() const { return pass_; }

 private:
  const VulkanDevice& device_;
  VkRenderPass pass_{VK_NULL_HANDLE};
};

}  // namespace gpu::vulkan
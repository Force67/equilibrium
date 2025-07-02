// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {
class VulkanDevice;

class VulkanCommandPool {
 public:
  explicit VulkanCommandPool(const VulkanDevice& dev, uint32_t queueFamilyIndex);
  ~VulkanCommandPool();
  VkCommandBuffer AllocatePrimary();
  void Reset();  // optional, for per-frame reset

  VkCommandPool handle() const { return pool_; }

 private:
  const VulkanDevice& device_;
  VkCommandPool pool_{VK_NULL_HANDLE};
};

}  // namespace gpu::vulkan

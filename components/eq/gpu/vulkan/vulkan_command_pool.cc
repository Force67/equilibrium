// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "vulkan_command_pool.h"
#include "vulkan_device.h"

namespace gpu::vulkan {
VulkanCommandPool::VulkanCommandPool(const VulkanDevice& dev, uint32_t queueFamilyIndex)
    : device_(dev) {
  VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  ci.queueFamilyIndex = queueFamilyIndex;

  BASE_BUGCHECK(vkCreateCommandPool(device_.handle(), &ci, nullptr, &pool_));
}

VulkanCommandPool::~VulkanCommandPool() {
  if (pool_ != VK_NULL_HANDLE)
    vkDestroyCommandPool(device_.handle(), pool_, nullptr);
}

VkCommandBuffer VulkanCommandPool::AllocatePrimary() {
  VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  ai.commandPool = pool_;
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ai.commandBufferCount = 1;

  VkCommandBuffer cmd{};
  BASE_BUGCHECK(vkAllocateCommandBuffers(device_.handle(), &ai, &cmd));
  return cmd;
}

void VulkanCommandPool::Reset() {
  vkResetCommandPool(device_.handle(), pool_, 0);
}

}  // namespace gpu::vulkan

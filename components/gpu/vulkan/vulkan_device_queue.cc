// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gpu/vulkan/vulkan_device_queue.h>

namespace gpu::vulkan {

bool VulkanDeviceQueue::Initialize() {


  float queuePriority = 0.f;
  VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = 0,  // fill me
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
  };

  return false;
}
}  // namespace gpu::vulkan
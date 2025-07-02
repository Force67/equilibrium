// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "vulkan_device.h"
#include <set>

#include <base/containers/vector.h>

namespace gpu::vulkan {

VulkanDevice::VulkanDevice(VkPhysicalDevice physical,
                           const QueueFamilyIndices& idx,
                           const std::vector<const char*>& extensions)
    : physical_(physical) {
  // Build unique set of family indices
  std::set<uint32_t> uniqueFamilies = {idx.graphics, idx.present, idx.compute,
                                       idx.transfer};

  float priority = 1.0f;
  base::Vector<VkDeviceQueueCreateInfo> qi(uniqueFamilies.size(),
                                           base::VectorReservePolicy::kForPushback);
  for (uint32_t family : uniqueFamilies) {
    VkDeviceQueueCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    ci.queueFamilyIndex = family;
    ci.queueCount = 1;
    ci.pQueuePriorities = &priority;
    qi.push_back(ci);
  }

  VkDeviceCreateInfo device_ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  device_ci.queueCreateInfoCount = static_cast<uint32_t>(qi.size());
  device_ci.pQueueCreateInfos = qi.data();
  device_ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  device_ci.ppEnabledExtensionNames = extensions.data();

  BASE_BUGCHECK(::vkCreateDevice(physical, &device_ci, nullptr, &device_) == VK_SUCCESS,
           "Failed to create Vulkan logical device");

  // Retrieve queues
  vkGetDeviceQueue(device_, idx.graphics, 0, &queues_.graphics);
  vkGetDeviceQueue(device_, idx.present, 0, &queues_.present);
  vkGetDeviceQueue(device_, idx.compute, 0, &queues_.compute);
  vkGetDeviceQueue(device_, idx.transfer, 0, &queues_.transfer);
}

VulkanDevice::~VulkanDevice() {
  if (device_ != VK_NULL_HANDLE) {
    vkDestroyDevice(device_, nullptr);
  }
}
}  // namespace gpu::vulkan
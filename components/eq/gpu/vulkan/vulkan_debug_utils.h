// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <eq/gpu/vulkan/vulkan_helpers.h>

namespace gpu::vulkan {
class VulkanInstance;

class DebugMessenger {
 public:
  DebugMessenger(VulkanInstance&);
  ~DebugMessenger();

 private:
  VulkanInstance& vulkan_instance_;
  VkDebugUtilsMessengerEXT debug_messenger_ext_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan

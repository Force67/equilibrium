// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vulkan/vulkan_raii.h>

namespace gpu::vulkan {

class VulkanDeviceQueue {
 public:
  VulkanDeviceQueue(VkInstance_Cxx& inst) : vk_instance_(inst){};

  bool Initialize();
 private:
  VkInstance_Cxx& vk_instance_;
  VkQueue present_queue_{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
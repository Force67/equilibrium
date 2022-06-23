// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/dynamic_library.h>
#include <vulkan/vulkan_helpers.h>

namespace gpu::vulkan {

class VulkanInstance {
 protected:
  explicit VulkanInstance(base::DynamicLibrary& library, VkInstance instance);

 public:
  ~VulkanInstance();

 private:
  base::DynamicLibrary vk_dll_;
  VkInstance vk_instance_;
};
}  // namespace gpu::vulkan
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO(Vince): very expensive header.
#include <base/logging.h>
#include <vulkan/loader/glad/include/glad/vulkan.h>

namespace gpu::vulkan {
struct VkInstance_Cxx {
  VkInstance_Cxx() = default;

  VkInstance_Cxx(const VkInstanceCreateInfo& instance_create_info) {
    Make(instance_create_info);
  }

  ~VkInstance_Cxx() {
    if (instance != VK_NULL_HANDLE)
      vkDestroyInstance(instance, nullptr);
  }

  inline void Make(const VkInstanceCreateInfo& instance_create_info) {
    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
      LOG_ERROR("vkCreateInstance() failed: {}", result);
    }
  }

  operator bool() { return instance != VK_NULL_HANDLE; }

  VkInstance instance{VK_NULL_HANDLE};
};
}  // namespace gpu::vulkan
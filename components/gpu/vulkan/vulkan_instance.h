// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/dynamic_library.h>
#include <base/memory/lazy_instance.h>

#include <base/containers/vector.h>

#include <gpu/vulkan/vulkan_helpers.h>
#include <gpu/vulkan/vulkan_debug_utils.h>

namespace gpu::vulkan {

class VulkanInstance {
 public:
  explicit VulkanInstance();
  ~VulkanInstance();

  bool Create();
  void Teardown();

  VkInstance& instance() { return vk_instance_; }

 private:
  void BindFunctionPointers();
  static GLADapiproc LoadSymbol(void* user_pointer, const char* symbol_name);

 private:
  base::DynamicLibrary vk_dll_;
  base::LazyInstance<DebugMessenger> messenger_;
  VkInstance vk_instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};

  PFN_vkGetInstanceProcAddr get_instance_proc_{nullptr};
  PFN_vkGetDeviceProcAddr get_device_proc_{nullptr};
};
}  // namespace gpu::vulkan
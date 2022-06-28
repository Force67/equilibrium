// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/dynamic_library.h>
#include <base/memory/lazy_instance.h>

#include <base/containers/vector.h>

#include <gpu/vulkan/vulkan_raii.h>
#include <gpu/vulkan/vulkan_helpers.h>
#include <gpu/vulkan/vulkan_debug_utils.h>

namespace gpu::vulkan {

class VulkanInstance {
 public:
  explicit VulkanInstance();
  ~VulkanInstance();

  bool Create();

  VkInstance& instance() { return vk_instance_.instance; }

 private:
  void BindFunctionPointers();
  static GLADapiproc LoadSymbol(void* user_pointer, const char* symbol_name);

 private:
  base::DynamicLibrary vk_dll_;  // must be first, so it gets unloaded LAST!
  VkInstance_Cxx vk_instance_;   // must be second, order of destruction
  base::LazyInstance<DebugMessenger> messenger_;
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};

  PFN_vkGetInstanceProcAddr get_instance_proc_{nullptr};
  PFN_vkGetDeviceProcAddr get_device_proc_{nullptr};
};
}  // namespace gpu::vulkan
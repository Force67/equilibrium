#pragma once

#include <base/arch.h>
#include <vulkan/loader/glad/include/glad/vulkan.h>

namespace gpu::vulkan {
struct VulkanInstanceFactory {
  VulkanInstanceFactory() = default;

  struct AppInfo {
    const char* app_name;
    u32 app_version;
  };

  void LoadVK(bool verbose_info, bool debug_mode, const AppInfo& info);

  VkInstance instance_;
};
}  // namespace gpu::vulkan
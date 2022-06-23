#pragma once

#include <base/arch.h>
#include <vulkan/loader/glad/include/glad/vulkan.h>

namespace gpu::vulkan {
class VulkanInstanceFactory {
 public:
  VulkanInstanceFactory() = default;

  struct AppInfo {
    const char* app_name;
    u32 app_version;
  };

  void LoadVK(bool verbose_info, bool debug_mode, const AppInfo& info);
};
}  // namespace gpu
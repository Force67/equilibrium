
#include <base/logging.h>
#include <base/dynamic_library.h>
#include <base/filesystem/path.h>
#include <base/strings/char_algorithms.h>

#include <vulkan/vulkan_instance_factory.h>

namespace gpu::vulkan {

static constexpr const char* kVulkanLibraryNames[] = {
#if defined(OS_WIN)
    "vulkan-1.dll",
    "vulkan.dll",
#elif defined(OS_LINUX)
    "libvulkan.so.1",
    "libvulkan.so",
#endif
};

bool LoadVulkanLibrary(base::DynamicLibrary& library) {
  for (const auto* c : kVulkanLibraryNames) {
    if (library.Load(c, true))
      break;
  }

  if (!library.loaded()) {
    return false;
  }

  return false;
}

// VulkanInstanceFactory
void VulkanInstanceFactory::LoadVK(bool verbose_mode,
                                   bool debug_mode,
                                   const AppInfo& info) {
  // step 0, fetch symbols that dont require any previously bound instances
  i32 vk_version = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
  if (vk_version == 0) {
    LOG_ERROR("Failed to load vulkan.");
    return;
  }

  if (verbose_mode) {
    LOG_INFO("Loaded vulkan {}", vk_version);
  }

  // https://github.com/yehuohan/ln-grahpics/blob/e56aa5b568dc8651cde439fd73354dfcbf2c9af0/egraphics/src/libvktor/vk/instance.cpp#L80
  VkResult ret;

  const VkApplicationInfo app_info{
      .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = info.app_name,
      .applicationVersion = info.app_version,
      .pEngineName = "Equilibrium",
      .engineVersion = info.app_version,
      .apiVersion = 0};

  const VkInstanceCreateInfo create_info{
      .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = 0,
      //.enabledExtensionCount = static_cast<uint32_t>(info.extensions.size()),
      //.ppEnabledExtensionNames = info.extensions.data(),
  };

  VkInstance vk_instance = nullptr;
  auto result = vkCreateInstance(&create_info, nullptr, &vk_instance);
  if (result != VK_SUCCESS) {
    LOG_ERROR("Failed to create vulkan instance: {}", VulkanResultToString(result));
    return;
  }

  vk_version = gladLoaderLoadVulkan(vk_instance, nullptr, nullptr);
  if (vk_version == 0) {
    LOG_ERROR("Failed to load vulkan with instance.");
    return;
  }

  if (debug_mode) {
#if 0
    const VkDebugUtilsMessengerCreateInfoEXT debug_ci = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = info.debug_callback,
        .pUserData = nullptr,
    };
#endif
  }

  instance_ = vk_instance;
}
}  // namespace gpu::vulkan
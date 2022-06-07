
#include <base/logging.h>
#include <vulkan/vulkan_instance_factory.h>

namespace gpu {

namespace {
const char* VulkanResultToString(VkResult result) {
  switch (result) {
    case VK_SUCCESS:
      return "Success";
    case VK_NOT_READY:
      return "A fence or query has not yet completed";
    case VK_TIMEOUT:
      return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
      return "An event is signaled";
    case VK_EVENT_RESET:
      return "An event is unsignaled";
    case VK_INCOMPLETE:
      return "A return array was too small for the result";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "A host memory allocation has failed";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "A device memory allocation has failed";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "Initialization of an object could not be completed for "
             "implementation-specific reasons";
    case VK_ERROR_DEVICE_LOST:
      return "The logical or physical device has been lost";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "Mapping of a memory object has failed";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "A requested layer is not present or could not be loaded";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "A requested extension is not supported";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "A requested feature is not supported";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "The requested version of Vulkan is not supported by the driver or is "
             "otherwise incompatible";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "Too many objects of the type have already been created";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "A requested format is not supported on this device";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "A surface is no longer available";
    case VK_SUBOPTIMAL_KHR:
      return "A swapchain no longer matches the surface properties exactly, but can "
             "still be used";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "A surface has changed in such a way that it is no longer compatible "
             "with the swapchain";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "The requested window is already connected to a VkSurfaceKHR, or to "
             "some other non-Vulkan API";
    default:
      return "ERROR: UNKNOWN VULKAN ERROR";
  }
}
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

  return vk_instance;
}
}  // namespace gpu
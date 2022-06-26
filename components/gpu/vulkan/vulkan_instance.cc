// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <vector>
#include <base/logging.h>
#include <gpu/vulkan/vulkan_loader.h>
#include <gpu/vulkan/vulkan_instance.h>

namespace gpu::vulkan {

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanErrorCallback(VkDebugReportFlagsEXT flags,
                    VkDebugReportObjectTypeEXT object_type,
                    uint64_t object,
                    size_t location,
                    int32_t message_code,
                    const char* layer_prefix,
                    const char* message,
                    void* user_data) {
  // todo, better reporting..
  LOG_ERROR("VK_ERROR: {}\n", message);
  return VK_FALSE;
}

// see
// https://github.com/glfw/glfw/blob/a465c1c32e0754d3de56e01c59a0fef33202f04c/src/win32_window.c#L2379
const char* kRequiredExtensions[] {
  "VK_KHR_surface",
#if defined(OS_WIN)
      "VK_KHR_win32_surface"
#endif
#if defined(CONFIG_DEBUG)
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

const char* kValidationLayers[]{"VK_LAYER_KHRONOS_validation"};
bool CheckValidationLayers() {
  u32 count;
  vkEnumerateInstanceLayerProperties(&count, nullptr);

  // vector is broken!
  base::Vector<VkLayerProperties> available_layers(
      static_cast<mem_size>(count), base::VectorReservePolicy::kForData);
  vkEnumerateInstanceLayerProperties(&count, available_layers.data());

  for (const char* layerName : kValidationLayers) {
    bool layerFound = false;
    for (const auto& layerProperties : available_layers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return false;
    }
  }
  return true;
}
}  // namespace

VulkanInstance::VulkanInstance() {}
VulkanInstance::~VulkanInstance() {
  Teardown();
}

extern "C" int glad_vulkan_is_device_function(const char* name);

GLADapiproc VulkanInstance::LoadSymbol(void* user_pointer, const char* symbol_name) {
  VulkanInstance* instance = nullptr;
  DCHECK(instance = reinterpret_cast<VulkanInstance*>(user_pointer));

  PFN_vkVoidFunction result = nullptr;
  if (instance->device_ != VK_NULL_HANDLE &&
      glad_vulkan_is_device_function(symbol_name))
    result = instance->get_device_proc_(instance->device_, symbol_name);

  if (result == nullptr && instance->vk_instance_ != VK_NULL_HANDLE)
    result = instance->get_instance_proc_(instance->vk_instance_, symbol_name);

  if (result == nullptr)
    result = instance->vk_dll_.FindSymbol<PFN_vkVoidFunction>(symbol_name);

  return result;
}

// needs to be called if a device is constructed too..
void VulkanInstance::BindFunctionPointers() {
  if (!get_instance_proc_)
    get_instance_proc_ =
        vk_dll_.FindSymbol<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  if (!get_device_proc_)
    get_device_proc_ =
        vk_dll_.FindSymbol<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

  if (get_instance_proc_ && get_device_proc_) {
    i32 version =
        gladLoadVulkanUserPtr(physical_device_, VulkanInstance::LoadSymbol, this);
  }
}

// NOTE(Vince): We are using GLAD for generating our vulkan loaders.
// see https://gen.glad.sh/

bool VulkanInstance::Create() {
  if (!LoadVulkanLibrary(vk_dll_)) {
    LOG_ERROR("Failed to load vulkan library");
    return false;
  }

  // initial bind
  BindFunctionPointers();

#if defined(CONFIG_DEBUG)
  {
    bool result = CheckValidationLayers();
    DCHECK(result);
    if (!result)
      return false;
  }
#endif

  const VkApplicationInfo app_info{
      .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "Equilibrium",
      .applicationVersion = 1337,
      .pEngineName = "Equilibrium/gpu",
      .engineVersion = 1337,
      .apiVersion = 0};

  const VkInstanceCreateInfo instance_create_info {
    .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr, .flags = 0, .pApplicationInfo = &app_info,
#if defined(CONFIG_DEBUG)
    .enabledLayerCount = sizeof(kValidationLayers) / sizeof(const char*),
    .ppEnabledLayerNames = kValidationLayers,
#else
    .enabledLayerCount = 0, .ppEnabledLayerNames = nullptr,
#endif
    .enabledExtensionCount = sizeof(kRequiredExtensions) / sizeof(const char*),
    .ppEnabledExtensionNames = kRequiredExtensions,
  };

  VkResult result = vkCreateInstance(&instance_create_info, nullptr, &vk_instance_);
  if (VK_SUCCESS != result) {
    LOG_ERROR("vkCreateInstance() failed: {}", result);
    return false;
  }

  // bind again with the instance
  BindFunctionPointers();

  return true;
}

void VulkanInstance::Teardown() {
  if (vk_instance_ != VK_NULL_HANDLE)
    vkDestroyInstance(vk_instance_, nullptr);
}
}  // namespace gpu::vulkan
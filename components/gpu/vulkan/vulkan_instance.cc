// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <vector>
#include <base/logging.h>
#include <gpu/vulkan/vulkan_loader.h>
#include <gpu/vulkan/vulkan_instance.h>

namespace gpu::vulkan {

namespace {

// see
// https://github.com/glfw/glfw/blob/a465c1c32e0754d3de56e01c59a0fef33202f04c/src/win32_window.c#L2379
constexpr const char* kRequiredExtensions[] {
  "VK_KHR_surface",
#if defined(OS_WIN)
      "VK_KHR_win32_surface",
#endif
#if defined(CONFIG_DEBUG)
      "VK_EXT_debug_utils",
#endif
};

bool CheckExtensions() {
  u32 count;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

  base::Vector<VkExtensionProperties> available_extensions(
      static_cast<mem_size>(count), base::VectorReservePolicy::kForData);
  vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                         available_extensions.data());

  for (const char* ext_name : kRequiredExtensions) {
    bool ext_found = false;
    for (const auto& ext : available_extensions) {
      if (strcmp(ext_name, ext.extensionName) == 0) {
        ext_found = true;
        break;
      }
    }
    if (!ext_found) {
      return false;
    }
  }

  return true;
}

constexpr const char* kValidationLayers[]{"VK_LAYER_KHRONOS_validation"};

bool CheckValidationLayers() {
  u32 count;
  vkEnumerateInstanceLayerProperties(&count, nullptr);

  base::Vector<VkLayerProperties> available_layers(
      static_cast<mem_size>(count), base::VectorReservePolicy::kForData);
  vkEnumerateInstanceLayerProperties(&count, available_layers.data());

  for (const char* layer_name : kValidationLayers) {
    bool layer_found = false;
    for (const auto& layer : available_layers) {
      if (strcmp(layer_name, layer.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      return false;
    }
  }
  return true;
}
}  // namespace

VulkanInstance::VulkanInstance() {}
VulkanInstance::~VulkanInstance() {
}

extern "C" int glad_vulkan_is_device_function(const char* name);

GLADapiproc VulkanInstance::LoadSymbol(void* user_pointer, const char* symbol_name) {
  VulkanInstance* instance = nullptr;
  DCHECK(instance = reinterpret_cast<VulkanInstance*>(user_pointer));

  PFN_vkVoidFunction result = nullptr;
  if (instance->device_ != VK_NULL_HANDLE &&
      glad_vulkan_is_device_function(symbol_name))
    result = instance->get_device_proc_(instance->device_, symbol_name);

  if (result == nullptr && instance->vk_instance_.instance != VK_NULL_HANDLE)
    result =
        instance->get_instance_proc_(instance->vk_instance_.instance, symbol_name);

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

    result = CheckExtensions();
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

  vk_instance_.Make(instance_create_info);
  if (!vk_instance_)
    return false;
  // bind again with the instance
  BindFunctionPointers();

#if defined(CONFIG_DEBUG)
  // attach the debug log
  messenger_.Make(*this);
#endif

  return true;
}
}  // namespace gpu::vulkan
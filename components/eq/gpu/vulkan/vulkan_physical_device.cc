// Copyright (C) 2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "vulkan_physical_device.h"
#include <algorithm>

namespace gpu::vulkan {

static std::vector<VkExtensionProperties> enumerateDeviceExtensions(
    VkPhysicalDevice physical) {
  uint32_t count = 0;
  vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> list(count);
  if (count > 0) {
    vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, list.data());
  }
  return list;
}

PhysicalDevice::PhysicalDevice(VkInstance instance) : instance_(instance) {}

bool PhysicalDevice::PickBest(std::vector<const char*> required_extensions,
                              VkSurfaceKHR surface) {
  // 1) Enumerate all GPUs
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance_, &count, nullptr);
  if (count == 0)
    return false;

  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance_, &count, devices.data());

  // 2) Score them (e.g. discrete > integrated, extension support, swapchain support)
  for (auto pd : devices) {
    VkPhysicalDeviceProperties p;
    VkPhysicalDeviceFeatures f;
    vkGetPhysicalDeviceProperties(pd, &p);
    vkGetPhysicalDeviceFeatures(pd, &f);

    // require extensions
    if (!PhysicalDevice{instance_}.supportsExtensions(pd, required_extensions))
      continue;

    // if a surface is given, we also need swapchain support
    if (surface != VK_NULL_HANDLE) {
      auto qfam = FindQueueFamilies(pd, surface);
      if (!qfam)
        continue;
    }

    // pick first that matches (you can add scoring logic here)
    device_ = pd;
    props_ = p;
    feats_ = f;
    qFamilies_ = FindQueueFamilies(pd, surface);
    return true;
  }

  return false;
}

bool PhysicalDevice::supportsExtensions(VkPhysicalDevice physical,
                                        const std::vector<const char*>& exts) const {
  auto avail = enumerateDeviceExtensions(physical);
  for (auto req : exts) {
    bool ok = std::any_of(avail.begin(), avail.end(),
                          [&](auto& e) { return strcmp(e.extensionName, req) == 0; });
    if (!ok)
      return false;
  }
  return true;
}

std::vector<VkExtensionProperties> PhysicalDevice::availableExtensions() const {
  if (device_ == VK_NULL_HANDLE) {
    return {};
  }
  return enumerateDeviceExtensions(device_);
}

std::optional<QueueFamilies> PhysicalDevice::FindQueueFamilies(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface) const {
  QueueFamilies indices;
  bool found_graphics = false, found_compute = false, found_transfer = false,
       found_present = false;

  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
  if (count == 0)
    return std::nullopt;
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, families.data());

  for (uint32_t i = 0; i < count; ++i) {
    const auto& prop = families[i];
    if (!found_graphics && (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      indices.graphics = i;
      found_graphics = true;
    }
    if (!found_compute && (prop.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      indices.compute = i;
      found_compute = true;
    }
    if (!found_transfer && (prop.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
      indices.transfer = i;
      found_transfer = true;
    }
    VkBool32 present = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &present);
    if (!found_present && present == VK_TRUE) {
      indices.present = i;
      found_present = true;
    }
    if (found_graphics && found_compute && found_transfer && found_present)
      break;
  }
  if (found_graphics && found_compute && found_transfer && found_present)
    return indices;
  return std::nullopt;
}
}  // namespace gpu::vulkan
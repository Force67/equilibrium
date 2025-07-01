// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "vulkan_surface.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "base/arch.h"
#include "vulkan_instance.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_LINUX)
#include <xcb/xcb.h>
#endif

namespace gpu::vulkan {

namespace {

bool CanPresent(VkInstance instance,
                VkPhysicalDevice physical_device,
                u32 queue_family_index,
                VkSurfaceKHR surface) {
  VkBool32 present_support = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface,
                                       &present_support);
  return present_support == VK_TRUE;
}

std::optional<u32> FindQueueFamily(VkInstance instance,
                                        VkPhysicalDevice physical_device,
                                        VkQueueFlags desired_flags,
                                        VkSurfaceKHR surface) {
  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           queue_families.data());

  for (u32 i = 0; i < queue_family_count; ++i) {
    bool supports_desired =
        (queue_families[i].queueFlags & desired_flags) == desired_flags;
    bool supports_present = CanPresent(instance, physical_device, i, surface);
    if (supports_desired && supports_present) {
      return i;
    }
  }
  // Fallback: find first that supports present.
  for (uint32_t i = 0; i < queue_family_count; ++i) {
    if (CanPresent(instance, physical_device, i, surface)) {
      return i;
    }
  }
  return std::nullopt;
}

}  // namespace

VulkanSurface::~VulkanSurface() {}

bool VulkanSurface::Initialize(void* os_hinstance,
                               void* os_window_handle,
                               Format format) {
  (void)format;  // Unused parameter.
#if defined(OS_WIN)
  VkWin32SurfaceCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .hinstance = static_cast<HINSTANCE>(os_hinstance),
      .hwnd = static_cast<HWND>(os_window_handle),
  };
  VkResult result =
      vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_handle_);
#elif defined(OS_LINUX)
  VkXcbSurfaceCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .connection = static_cast<xcb_connection_t*>(os_hinstance),
      .window = *static_cast<xcb_window_t*>(os_window_handle),
  };
  VkResult result =
      vkCreateXcbSurfaceKHR(instance_, &create_info, nullptr, &surface_handle_);
#elif defined(OS_MAC)
  VkMacOSSurfaceCreateInfoMVK create_info = {
      .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
      .pNext = nullptr,
      .flags = 0,
      .pView = os_window_handle,
  };
  VkResult result =
      vkCreateMacOSSurfaceMVK(instance_, &create_info, nullptr, &surface_handle_);
#else
#error Unsupported platform for VulkanSurface
#endif
  if (result != VK_SUCCESS) {
    return false;
  }

  // Query physical device and find queue family that supports graphics and
  // present. Assume the first enumerated physical device.
  u32 device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
  if (device_count == 0) {
    return false;
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

  VkPhysicalDevice physical_device = devices[0];
  std::optional<u32> queue_family =
      FindQueueFamily(instance_, physical_device, VK_QUEUE_GRAPHICS_BIT, surface_handle_);
  if (!queue_family.has_value()) {
    return false;
  }
  present_family_ = queue_family.value();

  return true;
}

}  // namespace gpu::vulkan
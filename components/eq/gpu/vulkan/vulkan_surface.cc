// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include "vulkan_surface.h"

namespace gpu::vulkan {
namespace {
bool CanPresent(VkInstance_Cxx& vki, VkPhysicalDevice phs, u32 queue_family_index) {
  return false;
}
}  // namespace

VulkanSurface::~VulkanSurface() {
  if (surface_handle_ != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance_, surface_handle_, nullptr);
  }
}

bool VulkanSurface::Initialize(void* os_hinstance, void* os_window_handle, Format format) {
#if defined(OS_WIN)
  const VkWin32SurfaceCreateInfoKHR win32_surface_create_info{
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .hinstance = static_cast<HINSTANCE>(os_hinstance),
      .hwnd = static_cast<HWND>(os_window_handle),
  };
  VkResult result = vkCreateWin32SurfaceKHR(
      instance_, &win32_surface_create_info,
                                            nullptr, &surface_handle_);
  return result == VK_SUCCESS;
#endif
}
}  // namespace gpu::vulkan
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <gpu/vulkan/vulkan_surface.h>

namespace gpu::vulkan {
namespace {
bool CanPresent(VkInstance_Cxx& vki, VkPhysicalDevice phs, u32 queue_family_index) {
  return false;
}
}  // namespace

VulkanSurface::VulkanSurface() {}
VulkanSurface::~VulkanSurface() {}

bool VulkanSurface::Initialize(VkInstance_Cxx& vki,
                               void* os_hinstance,
                               void* os_window_handle) {
  VkWin32SurfaceCreateInfoKHR surface_create_info{};
  surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_create_info.pNext = nullptr;
  surface_create_info.flags = 0;
  surface_create_info.hinstance = static_cast<HINSTANCE>(os_hinstance);
  surface_create_info.hwnd = static_cast<HWND>(os_window_handle);

  VkResult result = vkCreateWin32SurfaceKHR(vki.instance, &surface_create_info,
                                            nullptr, &surface_handle_);
  return result == VK_SUCCESS;
}
}  // namespace gpu::vulkan
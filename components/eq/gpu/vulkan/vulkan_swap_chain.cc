// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "vulkan_surface.h"
#include "vulkan_swap_chain.h"

namespace gpu::vulkan {
constexpr const char kDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VulkanSwapChain::~VulkanSwapChain() {
  if (swapchain_handle_ != VK_NULL_HANDLE) {
    // vkDestroySwapchainKHR()
  }
}

bool VulkanSwapChain::Create(VulkanSurface& surface,
                             VulkanDeviceQueue& device_queue,
                             u32 min_image_count,
                             const VkSurfaceFormatKHR& format,
                             const base::Vec2<u32> surface_size) {
  const VkSwapchainCreateInfoKHR swap_chain_create_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .flags = 0,
      .surface = surface.instance(),
      .minImageCount = min_image_count,
      .imageFormat = format.format,
      .imageColorSpace = format.colorSpace,
      .imageExtent = {surface_size.a, surface_size.b},  // size
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE};

  VkDevice device = VK_NULL_HANDLE;

  VkSwapchainKHR new_swap_chain = VK_NULL_HANDLE;
  auto result = vkCreateSwapchainKHR(device, &swap_chain_create_info,
                                     /*pAllocator=*/nullptr, &new_swap_chain);

  if (!new_swap_chain) {
    LOG_DEBUG("Failed to create a swapchain");
    return false;
  }

  swapchain_handle_ = new_swap_chain;
  return true;
}


}  // namespace gpu::vulkan
// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "gpu/vulkan/vulkan_swap_chain.h"

#include <vector>

#include <base/check.h>

#include "gpu/vulkan/vulkan_helpers.h"
#include "gpu/vulkan/vulkan_device.h"
#include "gpu/vulkan/vulkan_surface.h"

namespace gpu::vulkan {

VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, VulkanSurface& surface)
    : device_(device), surface_(surface) {}

VulkanSwapchain::~VulkanSwapchain() {
  for (VkImageView image_view : image_views_) {
    vkDestroyImageView(device_.handle(), image_view, nullptr);
  }
  if (swapchain_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device_.handle(), swapchain_, nullptr);
  }
}

bool VulkanSwapchain::Initialize(u32 width,
                                 u32 height,
                                 u32 graphics_queue_index,
                                 u32 present_queue_index) {
  QuerySupport();
  format_ = ChooseFormat();
  present_mode_ = ChoosePresentMode();
  extent_ = ChooseExtent(width, height);

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface_.instance();

  // Request one more image than the minimum to avoid waiting on the driver.
  u32 image_count = support_.capabilities.minImageCount + 1;
  if (support_.capabilities.maxImageCount > 0 &&
      image_count > support_.capabilities.maxImageCount) {
    image_count = support_.capabilities.maxImageCount;
  }
  create_info.minImageCount = image_count;

  create_info.imageFormat = format_.format;
  create_info.imageColorSpace = format_.colorSpace;
  create_info.imageExtent = extent_;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  u32 queue_family_indices[] = {graphics_queue_index, present_queue_index};
  if (graphics_queue_index != present_queue_index) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;      // Optional for exclusive mode
    create_info.pQueueFamilyIndices = nullptr;  // Optional for exclusive mode
  }

  create_info.preTransform = support_.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode_;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = swapchain_;  // VK_NULL_HANDLE on first creation

  BASE_DCHECK(vkCreateSwapchainKHR != nullptr);  // ensure the symbol was loaded (did you
                                                 // call BindFunctionPointers(device) ???)
  if (vkCreateSwapchainKHR(device_.handle(), &create_info, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    return false;
  }

  // Retrieve swap chain images.
  vkGetSwapchainImagesKHR(device_.handle(), swapchain_, &image_count, nullptr);
  images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_.handle(), swapchain_, &image_count, images_.data());

  // Create image views for each swap chain image.
  image_views_.resize(images_.size());
  for (size_t i = 0; i < images_.size(); ++i) {
    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = images_[i];
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = format_.format;
    view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    EQ_GPU_VK_BUGCHECK(vkCreateImageView(device_.handle(), &view_create_info, nullptr,
                                         &image_views_[i]));
  }

  return true;
}

void VulkanSwapchain::QuerySupport() {
  VkPhysicalDevice physical_device = device_.getPhysicalDevice();
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_.instance(),
                                            &support_.capabilities);

  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_.instance(),
                                       &format_count, nullptr);
  if (format_count != 0) {
    support_.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_.instance(),
                                         &format_count, support_.formats.data());
  }

  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_.instance(),
                                            &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    support_.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_.instance(),
                                              &present_mode_count,
                                              support_.present_modes.data());
  }
}

VkSurfaceFormatKHR VulkanSwapchain::ChooseFormat() const {
  for (const auto& available_format : support_.formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }
  // Return the first available format as a fallback.
  return support_.formats[0];
}

VkPresentModeKHR VulkanSwapchain::ChoosePresentMode() const {
  for (const auto& available_present_mode : support_.present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  // This mode is guaranteed to be available by the Vulkan specification.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::ChooseExtent(u32 width, u32 height) const {
  // If currentExtent is not UINT32_MAX, the window manager is controlling
  // the size, so we must use its value.
  if (support_.capabilities.currentExtent.width != UINT32_MAX) {
    return support_.capabilities.currentExtent;
  }

  // Otherwise, we can choose our own size, clamped to the allowed range.
  VkExtent2D actual_extent = {width, height};
  actual_extent.width =
      std::clamp(actual_extent.width, support_.capabilities.minImageExtent.width,
                 support_.capabilities.maxImageExtent.width);
  actual_extent.height =
      std::clamp(actual_extent.height, support_.capabilities.minImageExtent.height,
                 support_.capabilities.maxImageExtent.height);
  return actual_extent;
}

}  // namespace gpu::vulkan
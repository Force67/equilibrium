// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/math/vec2.h>

#include <eq/gpu/vulkan/vulkan_raii.h>
#include <vector>

namespace gpu::vulkan {

class VulkanSurface;
class VulkanDevice;

struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
  u32 formats_count;
  u32 present_count;
};

class VulkanSwapchain {
 public:
  VulkanSwapchain(VulkanDevice& device, VulkanSurface& surface);
  ~VulkanSwapchain();

  // Recreate on resize
  bool Initialize(u32 width, u32 height, u32 graphisqueueindice, u32 presentindice);

  VkSwapchainKHR get() const { return swapchain_; }
  VkFormat format() const { return format_.format; }
  VkColorSpaceKHR colorSpace() const { return format_.colorSpace; }
  VkExtent2D extent() const { return extent_; }
  const std::vector<VkImage>& images() const { return images_; }
  const std::vector<VkImageView>& imageViews() const { return image_views_; }

  auto image_format() const {
	return VkSurfaceFormatKHR{format_.format, format_.colorSpace};
  }

 private:
  void QuerySupport();
  VkSurfaceFormatKHR ChooseFormat() const;
  VkPresentModeKHR ChoosePresentMode() const;
  VkExtent2D ChooseExtent(u32 width, u32 height) const;

  VulkanDevice& device_;
  VulkanSurface& surface_;
  SwapchainSupportDetails support_;

  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  VkSurfaceFormatKHR format_{};
  VkPresentModeKHR present_mode_{VK_PRESENT_MODE_FIFO_KHR};
  VkExtent2D extent_{};
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;
};

}  // namespace gpu::vulkan
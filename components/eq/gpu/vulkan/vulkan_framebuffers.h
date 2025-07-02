// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

class VulkanDevice;
class VulkanRenderPass;
class VulkanSwapchain;

// Owns one VkFramebuffer per swap-chain image.
// Recreate whenever the swap-chain is recreated (resize / format change).
class VulkanFramebuffers {
 public:
  /** Build framebuffers for every image view in @p swapchain. */
  VulkanFramebuffers(const VulkanDevice& dev,
                     const VulkanRenderPass& renderPass,
                     const VulkanSwapchain& swapchain);
  ~VulkanFramebuffers();

  // Number of framebuffers (== swap-chain image count).
  uint32_t count() const { return static_cast<uint32_t>(framebuffers_.size()); }

  // Access framebuffer by image index.
  VkFramebuffer operator[](size_t i) const { return framebuffers_[i]; }

  // Width × height of the attachments.
  VkExtent2D extent() const { return extent_; }

 private:
  const VulkanDevice& device_;
  VkExtent2D extent_{};
  std::vector<VkFramebuffer> framebuffers_;
};

}  // namespace gpu::vulkan

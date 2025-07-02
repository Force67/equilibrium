// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

class VulkanDevice;

// Per-frame semaphores + fence bundle to drive the classic
// acquire->submit->present loop.
// Lifetime: whole application (recreated only if max-frames-in-flight changes).
class VulkanSyncObjects {
 public:
  explicit VulkanSyncObjects(const VulkanDevice& dev, uint32_t framesInFlight);
  ~VulkanSyncObjects();

  // Acquire the next swap-chain image & return its index.
  uint32_t BeginFrame(VkSwapchainKHR swapchain, uint32_t frameIdx);

  // Submit the recorded primary command buffer for execution.
  void Submit(VkCommandBuffer cmd, uint32_t frameIdx, VkQueue graphicsQueue);

  // Present the rendered image.
  void Present(uint32_t imageIndex,
               uint32_t frameIdx,
               VkQueue presentQueue,
               VkSwapchainKHR swapchain);

 private:
  struct FrameSync {
    VkSemaphore imageAvailable{VK_NULL_HANDLE};
    VkSemaphore renderFinished{VK_NULL_HANDLE};
    VkFence inFlight{VK_NULL_HANDLE};
  };

  const VulkanDevice& device_;
  uint32_t frameCount_{0};
  std::vector<FrameSync> frames_;
};

}  // namespace gpu::vulkan

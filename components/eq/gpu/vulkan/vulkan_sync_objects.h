// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

class VulkanDevice;

// Manages the synchronization primitives (semaphores and fences) required for
// rendering a set number of concurrent frames ("frames in flight").
// This class encapsulates the logic for the classic acquire-submit-present loop,
// ensuring that CPU and GPU operations are correctly ordered.
// Its lifetime typically matches the application's, recreated only if the
// maximum number of frames in flight is changed.
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

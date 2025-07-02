// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "vulkan_sync_objects.h"
#include "vulkan_device.h"

namespace gpu::vulkan {
VulkanSyncObjects::VulkanSyncObjects(const VulkanDevice& dev, uint32_t frames)
    : device_(dev), frameCount_(frames) {
  frames_.resize(frames);

  VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // so first frame doesn't stall

  for (auto& f : frames_) {
    BASE_BUGCHECK(vkCreateSemaphore(device_.handle(), &sci, nullptr, &f.imageAvailable));
    BASE_BUGCHECK(vkCreateSemaphore(device_.handle(), &sci, nullptr, &f.renderFinished));
    BASE_BUGCHECK(vkCreateFence(device_.handle(), &fci, nullptr, &f.inFlight));
  }
}

VulkanSyncObjects::~VulkanSyncObjects() {
  for (auto& f : frames_) {
    if (f.imageAvailable)
      vkDestroySemaphore(device_.handle(), f.imageAvailable, nullptr);
    if (f.renderFinished)
      vkDestroySemaphore(device_.handle(), f.renderFinished, nullptr);
    if (f.inFlight)
      vkDestroyFence(device_.handle(), f.inFlight, nullptr);
  }
}

uint32_t VulkanSyncObjects::BeginFrame(VkSwapchainKHR swapchain, uint32_t frameIndex) {
  auto& f = frames_[frameIndex];

  vkWaitForFences(device_.handle(), 1, &f.inFlight, VK_TRUE, UINT64_MAX);
  vkResetFences(device_.handle(), 1, &f.inFlight);

  uint32_t imageIndex = 0;
  VkResult res = vkAcquireNextImageKHR(device_.handle(), swapchain, UINT64_MAX,
                                       f.imageAvailable, VK_NULL_HANDLE, &imageIndex);
  /* handle resize etc. in caller */
  BASE_BUGCHECK(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);

  return imageIndex;
}

void VulkanSyncObjects::Submit(VkCommandBuffer cmd,
                               uint32_t frameIndex,
                               VkQueue graphicsQueue) {
  auto& f = frames_[frameIndex];

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.waitSemaphoreCount = 1;
  si.pWaitSemaphores = &f.imageAvailable;
  si.pWaitDstStageMask = &waitStage;
  si.commandBufferCount = 1;
  si.pCommandBuffers = &cmd;
  si.signalSemaphoreCount = 1;
  si.pSignalSemaphores = &f.renderFinished;

  BASE_BUGCHECK(vkQueueSubmit(graphicsQueue, 1, &si, f.inFlight));
}

void VulkanSyncObjects::Present(uint32_t imageIndex,
                                uint32_t frameIndex,
                                VkQueue presentQueue,
                                VkSwapchainKHR swapchain) {
  auto& f = frames_[frameIndex];

  VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  pi.waitSemaphoreCount = 1;
  pi.pWaitSemaphores = &f.renderFinished;
  pi.swapchainCount = 1;
  pi.pSwapchains = &swapchain;
  pi.pImageIndices = &imageIndex;

  vkQueuePresentKHR(presentQueue, &pi);
}

}  // namespace gpu::vulkan

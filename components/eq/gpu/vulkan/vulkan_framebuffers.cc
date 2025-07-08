// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "vulkan_framebuffers.h"
#include "vulkan_device.h"
#include "vulkan_swap_chain.h"
#include "vulkan_render_pass.h"

namespace gpu::vulkan {
VulkanFramebuffers::VulkanFramebuffers(const VulkanDevice& dev,
                                       const VulkanRenderPass& rp,
                                       const VulkanSwapchain& sc)
    : device_(dev), extent_(sc.extent()) {
  framebuffers_.resize(sc.images().size());

  for (size_t i = 0; i < framebuffers_.size(); ++i) {
    VkImageView attachments[] = {sc.imageViews()[i]};

    VkFramebufferCreateInfo fci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fci.renderPass = rp.handle();
    fci.attachmentCount = 1;
    fci.pAttachments = attachments;
    fci.width = extent_.width;
    fci.height = extent_.height;
    fci.layers = 1;

    BASE_BUGCHECK(vkCreateFramebuffer(device_.handle(), &fci, nullptr,
                                      &framebuffers_[i]) == VK_SUCCESS);
  }
}

VulkanFramebuffers::~VulkanFramebuffers() {
  for (auto fb : framebuffers_) {
    if (fb != VK_NULL_HANDLE)
      vkDestroyFramebuffer(device_.handle(), fb, nullptr);
  }
}

}  // namespace gpu::vulkan

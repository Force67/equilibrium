// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

class VulkanDevice;
class VulkanRenderPass;

class VulkanPipeline {
 public:
  VulkanPipeline(const VulkanDevice& dev,
                 const VulkanRenderPass& rp,
                 const std::vector<uint32_t>& vert,
                 const std::vector<uint32_t>& frag);
  ~VulkanPipeline();

  void Bind(VkCommandBuffer cmd) const;

  VkPipeline pipeline() const { return pipeline_; }
  VkPipelineLayout layout() const { return layout_; }

 private:
  const VulkanDevice& device_;
  VkPipelineLayout layout_{VK_NULL_HANDLE};
  VkPipeline pipeline_{VK_NULL_HANDLE};
};

}  // namespace gpu::vulkan

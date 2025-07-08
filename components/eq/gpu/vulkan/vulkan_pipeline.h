// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/span.h>
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

class VulkanDevice;
class VulkanRenderPass;

// A pipeline in Vulkan represents a collection of shaders and fixed-function
// configuration (vertex input, assembly, viewport, rasterization, etc.).
// This class manages a graphics pipeline's lifecycle, including layout creation
// for resource binding (descriptor sets, push constants) and pipeline execution.
// Note: Dynamic states (e.g., viewport/scissor) are not yet supported.
class VulkanPipeline {
 public:
  VulkanPipeline(const VulkanDevice& dev,
                 const VulkanRenderPass& rp,
                 const base::Span<uint32_t>& vert,
                 const base::Span<uint32_t>& frag);
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

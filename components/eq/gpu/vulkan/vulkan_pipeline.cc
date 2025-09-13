// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include "vulkan_pipeline.h"
#include "vulkan_device.h"
#include "vulkan_render_pass.h"
#include "vulkan_helpers.h"

namespace gpu::vulkan {
namespace {
VkShaderModule MakeShader(VkDevice dev, const base::Span<uint32_t>& code) {
  VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  ci.codeSize = code.size() * sizeof(uint32_t);
  ci.pCode = code.data();

  VkShaderModule mod{};
  EQ_GPU_VK_BUGCHECK(vkCreateShaderModule(dev, &ci, nullptr, &mod));
  return mod;
}
}  // namespace

VulkanPipeline::VulkanPipeline(const VulkanDevice& dev,
                               const VulkanRenderPass& rp,
                               const base::Span<uint32_t>& vertSpv,
                               const base::Span<uint32_t>& fragSpv)
    : device_(dev) {
  /* 1. shader stages ------------------------------------------------------- */
  VkShaderModule vert = MakeShader(device_.handle(), vertSpv);
  VkShaderModule frag = MakeShader(device_.handle(), fragSpv);

  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vert;
  stages[0].pName = "main";

  stages[1] = stages[0];
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = frag;

  /* 2. fixed-function state ------------------------------------------------ */
  VkPipelineVertexInputStateCreateInfo vi{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkPipelineInputAssemblyStateCreateInfo ia{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkViewport vp{};  // dynamic
  VkRect2D sc{};    // dynamic

  VkPipelineViewportStateCreateInfo vps{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  vps.viewportCount = 1;
  vps.pViewports = &vp;
  vps.scissorCount = 1;
  vps.pScissors = &sc;

  VkPipelineRasterizationStateCreateInfo rs{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rs.lineWidth = 1.f;
  rs.cullMode = VK_CULL_MODE_BACK_BIT;
  // Vulkan has inverted Y compared to GL; with a positive-height viewport
  // our shader's typical CCW triangle appears CW. Treat CW as front to avoid culling.
  rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rs.polygonMode = VK_POLYGON_MODE_FILL;

  VkPipelineMultisampleStateCreateInfo ms{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState blendAtt{};
  blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo cb{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  cb.attachmentCount = 1;
  cb.pAttachments = &blendAtt;

  VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dyn{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dyn.dynamicStateCount = sizeof(dynStates) / sizeof(VkDynamicState);
  dyn.pDynamicStates = dynStates;

  /* 3. pipeline layout ----------------------------------------------------- */
  VkPipelineLayoutCreateInfo lci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  EQ_GPU_VK_BUGCHECK(vkCreatePipelineLayout(device_.handle(), &lci, nullptr, &layout_));

  /* 4. graphics pipeline --------------------------------------------------- */
  VkGraphicsPipelineCreateInfo gpi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  gpi.stageCount = 2;
  gpi.pStages = stages;
  gpi.pVertexInputState = &vi;
  gpi.pInputAssemblyState = &ia;
  gpi.pViewportState = &vps;
  gpi.pRasterizationState = &rs;
  gpi.pMultisampleState = &ms;
  gpi.pColorBlendState = &cb;
  gpi.pDynamicState = &dyn;
  gpi.layout = layout_;
  gpi.renderPass = rp.handle();
  gpi.subpass = 0;

  EQ_GPU_VK_BUGCHECK(vkCreateGraphicsPipelines(device_.handle(), VK_NULL_HANDLE, 1, &gpi,
                                          nullptr, &pipeline_));

  /* shaders live only while creating the pipeline */
  vkDestroyShaderModule(device_.handle(), vert, nullptr);
  vkDestroyShaderModule(device_.handle(), frag, nullptr);
}

VulkanPipeline::~VulkanPipeline() {
  if (pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(device_.handle(), pipeline_, nullptr);
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(device_.handle(), layout_, nullptr);
}

void VulkanPipeline::Bind(VkCommandBuffer cmd) const {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

}  // namespace gpu::vulkan

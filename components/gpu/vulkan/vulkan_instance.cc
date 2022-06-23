// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gpu/vulkan/vulkan_instance.h>

namespace gpu::vulkan {

VulkanInstance::VulkanInstance(base::DynamicLibrary& library, VkInstance instance)
    : vk_dll_(library), vk_instance_(instance) {}
VulkanInstance::~VulkanInstance() {}



}  // namespace gpu::vulkan
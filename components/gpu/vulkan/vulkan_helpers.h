// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO(Vince): very expensive header.
#include <vulkan/vulkan.h>

namespace gpu::vulkan {

const char* VulkanResultToString(VkResult result);
}

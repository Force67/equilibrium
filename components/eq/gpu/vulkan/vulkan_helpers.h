// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO(Vince): very expensive header.
#include <eq/gpu/vulkan/vulkan.h>

namespace gpu::vulkan {

const char* VulkanResultToString(VkResult result);
}

#define EQ_GPU_VK_BUGCHECK(expr)                                           \
  do {                                                                     \
    VkResult res = (expr);                                                 \
    if (res != VK_SUCCESS) {                                               \
      BASE_BUGCHECK(false, "Vulkan error: %s", VulkanResultToString(res)); \
    }                                                                      \
  } while (0)

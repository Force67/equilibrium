// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/dynamic_library.h>

namespace gpu::vulkan {

bool LoadVulkanLibrary(base::DynamicLibrary& out_vkdll);
}  // namespace gpu::vulkan
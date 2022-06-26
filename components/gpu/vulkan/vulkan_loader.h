#pragma once

#include <base/arch.h>
#include <base/dynamic_library.h>

namespace gpu::vulkan {

bool LoadVulkanLibrary(base::DynamicLibrary& out_vkdll);
}  // namespace gpu::vulkan
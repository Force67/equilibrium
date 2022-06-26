
#include <base/logging.h>
#include <base/dynamic_library.h>
#include <base/filesystem/path.h>

namespace gpu::vulkan {

static constexpr const char* kVulkanLibraryNames[] = {
#if defined(OS_WIN)
    "vulkan-1.dll",
    "vulkan.dll",
#elif defined(OS_LINUX)
    "libvulkan.so.1",
    "libvulkan.so",
#endif
};

bool LoadVulkanLibrary(base::DynamicLibrary& out_vkdll) {
  for (const auto* c : kVulkanLibraryNames) {
    if (out_vkdll.Load(c, true))
      break;
  }

  return out_vkdll.loaded();
}
}  // namespace gpu::vulkan
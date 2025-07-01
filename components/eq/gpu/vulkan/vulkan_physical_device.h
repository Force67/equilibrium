// Copyright (C) 2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <eq/gpu/vulkan/vulkan.h>

#include <vector>
#include <optional>

namespace gpu::vulkan {

struct QueueFamilies {
  uint32_t graphics;
  uint32_t compute;
  uint32_t transfer;
  uint32_t present;
};

class PhysicalDevice {
 public:
  PhysicalDevice(VkInstance instance);
  ~PhysicalDevice() = default;

  // Pick the “best” GPU (e.g. discrete pref, required extensions).
  bool PickBest(std::vector<const char*> required_extensions,
                VkSurfaceKHR surface = VK_NULL_HANDLE);

  // Once picked, query properties/capabilities.
  VkPhysicalDeviceProperties properties() const { return props_; }
  VkPhysicalDeviceFeatures features() const { return feats_; }
  QueueFamilies queues() const { return qFamilies_.value(); }

  // Helpers
  bool supportsExtensions(VkPhysicalDevice physical,
                          const std::vector<const char*>& exts) const;
  std::vector<VkExtensionProperties> availableExtensions() const;

  VkPhysicalDevice handle() const { return device_; }

 private:
  VkInstance instance_;
  VkPhysicalDevice device_{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties props_{};
  VkPhysicalDeviceFeatures feats_{};
  std::optional<QueueFamilies> qFamilies_;

  std::optional<QueueFamilies> FindQueueFamilies(VkPhysicalDevice,
                                                 VkSurfaceKHR surface) const;
};

}  // namespace gpu::vulkan

// Copyright (C) 2022-2025 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <eq/gpu/vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace gpu::vulkan {

// Holds queue-family indices (uint32_t)
struct QueueFamilyIndices {
  u32 graphics;
  u32 present;
  u32 compute;
  u32 transfer;
};

// Holds VkQueue handles
struct DeviceQueues {
  VkQueue graphics;
  VkQueue present;
  VkQueue compute;
  VkQueue transfer;
};

class VulkanDevice {
 public:
  // physical: the VkPhysicalDevice handle
  // families: the indices of the queue families to create
  // extensions: device extensions to enable (e.g. swapchain)
  VulkanDevice(VkPhysicalDevice physical,
               const QueueFamilyIndices& families,
               const std::vector<const char*>& extensions);
  ~VulkanDevice();

  VulkanDevice(const VulkanDevice&) = delete;
  VulkanDevice& operator=(const VulkanDevice&) = delete;

  VkDevice handle() const { return device_; }
  VkPhysicalDevice getPhysicalDevice() const { return physical_; }
  const DeviceQueues& queues() const { return queues_; }

 private:
  VkDevice device_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_{VK_NULL_HANDLE};
  DeviceQueues queues_{};
};
}  // namespace gpu::vulkan

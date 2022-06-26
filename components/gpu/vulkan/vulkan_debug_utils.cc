// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>

#include <gpu/vulkan/vulkan_instance.h>
#include <gpu/vulkan/vulkan_debug_utils.h>

// view: https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanDebug.cpp
namespace gpu::vulkan {
namespace {
base::LogLevel TranslateLogLevel(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity) noexcept {
  switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      return base::LogLevel::kTrace;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      return base::LogLevel::kInfo;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      return base::LogLevel::kWarning;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      return base::LogLevel::kError;
    default:
      return base::LogLevel::kDebug;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                       void* pUserData) {
  const auto translated_level = TranslateLogLevel(message_severity);
  base::PrintLogMessage(translated_level, "[vk_debug_messenger]: {}\n",
                        pCallbackData->pMessage);
  return VK_FALSE;
}
}  // namespace

DebugMessenger::DebugMessenger(VulkanInstance& instance)
    : vulkan_instance_(instance) {
  const VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{
      .sType =
          VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = DebugMessengerCallback,
      .pUserData = this,
  };

  auto result = vkCreateDebugUtilsMessengerEXT(vulkan_instance_.instance(),
                                               &messenger_create_info, nullptr,
                                               &debug_messenger_ext_);

  if (result != VK_SUCCESS) {
    LOG_ERROR("Failed to create debug utils messenger: {}", result);
    return;
  }
}

DebugMessenger::~DebugMessenger() {
    // crash here???
  if (debug_messenger_ext_ != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(vulkan_instance_.instance(),
                                    debug_messenger_ext_, nullptr);
}
}  // namespace gpu::vulkan

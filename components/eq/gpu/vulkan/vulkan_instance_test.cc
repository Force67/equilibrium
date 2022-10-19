// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include "vulkan_instance.h"
#include "vulkan_surface.h"

namespace {
using namespace gpu::vulkan;

class VulkanTestSuite : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    instance_ = new VulkanInstance();
    ASSERT_TRUE(instance_);
    ASSERT_TRUE(instance_->Create()) << "Vulkan instance creation failed";
  }
  static void TearDownTestCase() { delete instance_; }

  auto& instance() { return *instance_; }

 private:
  static VulkanInstance* instance_;
};

VulkanInstance* VulkanTestSuite::instance_ = nullptr;

TEST_F(VulkanTestSuite, CreateSurface) {
  gpu::vulkan::VulkanSurface surface(this->instance().instance());
  ASSERT_TRUE(surface.Initialize(::GetModuleHandleW(nullptr), ::GetDesktopWindow(),
                                 VulkanSurface::Format::Default));
}
}  // namespace
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <gpu/vulkan/vulkan_instance.h>

namespace {
class VulkanInstanceTestSuite : public ::testing::Test {
 public:
  auto& instance() { return instance_; }
 private:
  gpu::vulkan::VulkanInstance instance_;
};

TEST_F(VulkanInstanceTestSuite, Create) {
  EXPECT_TRUE(instance().Create());
}
}  // namespace
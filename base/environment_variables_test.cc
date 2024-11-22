// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/environment_variables.h>

namespace {
TEST(EnvironmentVariables, SetReceive) {
  ASSERT_TRUE(base::SetEnvironmentVariable(u8"base_unitttest_env_var", u8"test"));

  base::StringU8 result;
  ASSERT_TRUE(base::GetEnvironmentVariable(u8"base_unitttest_env_var", result));
  ASSERT_EQ(result.length(), 4);
  ASSERT_TRUE(result == u8"test");

  ASSERT_TRUE(base::DeleteEnvironmentVariable(u8"base_unitttest_env_var"));
  ASSERT_FALSE(base::GetEnvironmentVariable(u8"base_unitttest_env_var", result));
}
}  // namespace
// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/option.h>
#include <base/environment_variables.h>
#include <base/strings/xstring.h>

#include <gtest/gtest.h>

namespace {

base::Option<bool> kFlag{"opt.flag", false, "BASE_OPT_FLAG", "a boolean"};
base::Option<int> kCount{"opt.count", 7, "BASE_OPT_COUNT", "a number"};
base::Option<float> kScale{"opt.scale", 1.0f, "BASE_OPT_SCALE"};
base::Option<const char*> kName{"opt.name", "default", "BASE_OPT_NAME"};

class OptionTest : public ::testing::Test {
 protected:
  void SetUp() override { TearDown(); }
  void TearDown() override {
    kFlag.Reset();
    kCount.Reset();
    kScale.Reset();
    kName.Reset();
    base::DeleteEnvironmentVariable(u8"BASE_OPT_FLAG");
    base::DeleteEnvironmentVariable(u8"BASE_OPT_COUNT");
    base::DeleteEnvironmentVariable(u8"BASE_OPT_SCALE");
    base::DeleteEnvironmentVariable(u8"BASE_OPT_NAME");
  }
};

TEST_F(OptionTest, ParsesEachType) {
  EXPECT_TRUE(kFlag.SetFromString("on"));
  EXPECT_TRUE(kCount.SetFromString("128"));
  EXPECT_TRUE(kScale.SetFromString("0.25"));
  EXPECT_TRUE(kFlag.get());
  EXPECT_EQ(kCount.get(), 128);
  EXPECT_FLOAT_EQ(kScale.get(), 0.25f);
}

// The whole reason a string option copies: SetFromString is handed whatever
// its caller had at the time, and the option is read long after that is gone.
TEST_F(OptionTest, StringValueOutlivesTheCallersBuffer) {
  {
    base::String text("from-a-temporary");
    ASSERT_TRUE(kName.SetFromString(text.c_str()));
  }
  EXPECT_STREQ(kName.get(), "from-a-temporary");
}

// InitOptionsFromEnv reads each variable into a local, which is the same trap
// one call further out.
TEST_F(OptionTest, EnvironmentOverrideOutlivesTheRead) {
  ASSERT_TRUE(base::SetEnvironmentVariable(u8"BASE_OPT_NAME", u8"from-the-env"));
  ASSERT_TRUE(base::SetEnvironmentVariable(u8"BASE_OPT_COUNT", u8"42"));

  EXPECT_GE(base::InitOptionsFromEnv(), 2u);
  EXPECT_STREQ(kName.get(), "from-the-env");
  EXPECT_EQ(kCount.get(), 42);
  EXPECT_TRUE(kName.overridden());
}

TEST_F(OptionTest, ResetDropsTheOwnedString) {
  {
    base::String text("temporary");
    ASSERT_TRUE(kName.SetFromString(text.c_str()));
  }
  kName.Reset();
  EXPECT_STREQ(kName.get(), "default");
  EXPECT_FALSE(kName.overridden());
}

TEST_F(OptionTest, RejectsWhatDoesNotParse) {
  ASSERT_TRUE(kCount.SetFromString("9"));
  EXPECT_FALSE(kCount.SetFromString("not-a-number"));
  EXPECT_EQ(kCount.get(), 9);  // a bad value leaves the old one alone
  EXPECT_FALSE(kName.SetFromString(nullptr));
}

// A string option pays for its buffer; nothing else should.
TEST_F(OptionTest, NonStringOptionsCarryNoStorage) {
  EXPECT_EQ(sizeof(base::Option<int>), sizeof(base::Option<unsigned>));
  EXPECT_LT(sizeof(base::Option<int>), sizeof(base::Option<const char*>));
}

}  // namespace

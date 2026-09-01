// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/option.h>
#include <base/option_file.h>
#include <base/filesystem/scoped_temp_dir.h>
#include <base/strings/xstring.h>

#include <gtest/gtest.h>

namespace {

base::Option<bool> kFlag{"test.flag", false, "BASE_TEST_FLAG", "a boolean"};
base::Option<int> kCount{"test.count", 7, "BASE_TEST_COUNT", "a number"};
base::Option<const char*> kName{"test.name", "default", "BASE_TEST_NAME"};

// The same knob, declared a second time the way a second module would.
base::Option<int> kCountElsewhere{"test.count", 7, "BASE_TEST_COUNT"};

class OptionFileTest : public ::testing::Test {
 protected:
  void SetUp() override { TearDown(); }
  void TearDown() override {
    kFlag.Reset();
    kCount.Reset();
    kCountElsewhere.Reset();
    kName.Reset();
  }
};

TEST_F(OptionFileTest, AppliesEntries) {
  const auto result = base::ApplyOptionText(
      "+test.flag=1\n"
      "+test.count=42\n");

  EXPECT_EQ(result.applied, 2u);
  EXPECT_EQ(result.unknown, 0u);
  EXPECT_EQ(result.invalid, 0u);
  EXPECT_TRUE(kFlag.get());
  EXPECT_EQ(kCount.get(), 42);
}

TEST_F(OptionFileTest, ReadsBoolsWrittenAsNumbers) {
  EXPECT_EQ(base::ApplyOptionText("+test.flag=2").applied, 1u);
  EXPECT_TRUE(kFlag.get());

  EXPECT_EQ(base::ApplyOptionText("+test.flag=nonsense").invalid, 1u);
  EXPECT_TRUE(kFlag.get());
}

TEST_F(OptionFileTest, MatchesEnvironmentVariableName) {
  EXPECT_EQ(base::ApplyOptionText("+BASE_TEST_COUNT=3").applied, 1u);
  EXPECT_EQ(kCount.get(), 3);
}

TEST_F(OptionFileTest, PlusPrefixIsOptional) {
  EXPECT_EQ(base::ApplyOptionText("test.count=5").applied, 1u);
  EXPECT_EQ(kCount.get(), 5);
}

TEST_F(OptionFileTest, BareNameSetsOne) {
  EXPECT_EQ(base::ApplyOptionText("+test.flag").applied, 1u);
  EXPECT_TRUE(kFlag.get());
}

TEST_F(OptionFileTest, MinusRestoresDefault) {
  base::ApplyOptionText("+test.count=99");
  ASSERT_EQ(kCount.get(), 99);

  EXPECT_EQ(base::ApplyOptionText("-test.count").applied, 1u);
  EXPECT_EQ(kCount.get(), 7);
  EXPECT_FALSE(kCount.overridden());
}

TEST_F(OptionFileTest, SkipsCommentsAndBlankLines) {
  const auto result = base::ApplyOptionText(
      "// a comment\n"
      "# another one\n"
      "\n"
      "   \n"
      "+test.count=1\n");

  EXPECT_EQ(result.applied, 1u);
  EXPECT_EQ(result.invalid, 0u);
}

TEST_F(OptionFileTest, TrimsAndUnquotes) {
  EXPECT_EQ(base::ApplyOptionText("  +test.name = \"a value\"  ").applied, 1u);
  EXPECT_STREQ(kName.get(), "a value");
}

TEST_F(OptionFileTest, LastEntryWins) {
  base::ApplyOptionText("+test.count=1\n+test.count=2\n");
  EXPECT_EQ(kCount.get(), 2);
}

TEST_F(OptionFileTest, SetsEveryOptionGoingByTheName) {
  EXPECT_EQ(base::ApplyOptionText("+test.count=13").applied, 1u);
  EXPECT_EQ(kCount.get(), 13);
  EXPECT_EQ(kCountElsewhere.get(), 13);

  EXPECT_TRUE(base::SetOptionValue("test.count", "14"));
  EXPECT_EQ(kCountElsewhere.get(), 14);
}

TEST_F(OptionFileTest, FillUnsetLeavesAlreadySetOptionsAlone) {
  base::ApplyOptionText("+test.count=5");

  const auto result = base::ApplyOptionText(
      "+test.count=9\n"
      "+test.name=from-defaults\n",
      base::OptionApply::kFillUnset);

  EXPECT_EQ(result.skipped, 1u);
  EXPECT_EQ(result.applied, 1u);
  EXPECT_EQ(kCount.get(), 5);
  EXPECT_STREQ(kName.get(), "from-defaults");
}

TEST_F(OptionFileTest, CountsUnknownNames) {
  const auto result = base::ApplyOptionText("+test.nosuchoption=1");
  EXPECT_EQ(result.unknown, 1u);
  EXPECT_EQ(result.applied, 0u);
}

TEST_F(OptionFileTest, CountsUnparsableValuesAndStrayLines) {
  const auto result = base::ApplyOptionText(
      "+test.count=notanumber\n"
      "test.count\n"
      "=5\n");

  EXPECT_EQ(result.invalid, 3u);
  EXPECT_EQ(result.applied, 0u);
  EXPECT_EQ(kCount.get(), 7);
}

// Option<const char*> binds to the string it is given, so a value read out of a
// file has to survive the buffer it was parsed from.
TEST_F(OptionFileTest, StringValueOutlivesSourceText) {
  {
    base::String text("+test.name=from-a-temporary");
    ASSERT_EQ(base::ApplyOptionText(text).applied, 1u);
  }
  EXPECT_STREQ(kName.get(), "from-a-temporary");
}

TEST_F(OptionFileTest, ReadsAndWritesFiles) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path path = dir.path() / "options.txt";

  EXPECT_FALSE(base::ApplyOptionFile(dir.path() / "missing.txt").read);

  base::ApplyOptionText("+test.count=23\n");
  ASSERT_TRUE(base::WriteOptionFile(path, /*overridden_only=*/true));
  kCount.Reset();

  const auto result = base::ApplyOptionFile(path);
  EXPECT_TRUE(result.read);
  EXPECT_EQ(result.invalid, 0u);
  EXPECT_EQ(kCount.get(), 23);
}

TEST_F(OptionFileTest, DumpsOverriddenOptionsAndReadsThemBack) {
  base::ApplyOptionText("+test.count=11\n+test.name=dumped\n");

  base::String dump;
  base::AppendOptionText(dump, /*overridden_only=*/true);
  kCount.Reset();
  kName.Reset();
  ASSERT_EQ(kCount.get(), 7);

  const auto result = base::ApplyOptionText(dump);
  EXPECT_EQ(result.invalid, 0u);
  EXPECT_EQ(kCount.get(), 11);
  EXPECT_STREQ(kName.get(), "dumped");
}

}  // namespace

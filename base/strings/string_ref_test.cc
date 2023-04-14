// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/string_ref.h>

namespace {
    using namespace base;

TEST(BasicStringRefTest, Construction) {
  std::string test_str = "Hello, world!";
  StringRef str_ref(test_str);

  EXPECT_EQ(str_ref.length(), test_str.length());
  EXPECT_EQ(str_ref.data(), test_str.data());
  EXPECT_TRUE(str_ref.IsNullTerminated());
}

TEST(BasicStringRefTest, NullTermination) {
  const char test_data[] = {'H', 'e', 'l', 'l', 'o'};
  StringRef str_ref(test_data, sizeof(test_data));

  EXPECT_EQ(str_ref.length(), sizeof(test_data));
  EXPECT_FALSE(str_ref.IsNullTerminated());
}

TEST(BasicStringRefTest, OperatorBrackets) {
  std::string test_str = "Hello, world!";
  StringRef str_ref(test_str);

  for (size_t i = 0; i < test_str.length(); ++i) {
    EXPECT_EQ(str_ref[i], test_str[i]);
  }
}

TEST(BasicStringRefTest, ComparisonOperators) {
  StringRef str_ref1("Hello, world!");
  StringRef str_ref2("Hello, world!");
  StringRef str_ref3("Goodbye, world!");

  EXPECT_TRUE(str_ref1 == str_ref2);
  EXPECT_FALSE(str_ref1 != str_ref2);
  EXPECT_FALSE(str_ref1 == str_ref3);
  EXPECT_TRUE(str_ref1 != str_ref3);

  EXPECT_TRUE(str_ref1 <= str_ref2);
  EXPECT_TRUE(str_ref1 >= str_ref2);

  EXPECT_TRUE(str_ref1 < str_ref3);
  EXPECT_TRUE(str_ref1 <= str_ref3);
  EXPECT_TRUE(str_ref3 > str_ref1);
  EXPECT_TRUE(str_ref3 >= str_ref1);
}

TEST(BasicStringRefTest, Find) {
  StringRef str_ref("Hello, world!");

  EXPECT_EQ(str_ref.find("world", 0, 5), 7u);
  EXPECT_EQ(str_ref.find("not_found", 0, 9), StringRef::npos);
}
}  // namespace